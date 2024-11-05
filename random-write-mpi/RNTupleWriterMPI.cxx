// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterMPI.hxx"

#include <ROOT/RMiniFile.hxx>
#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleMetrics.hxx>
#include <ROOT/RNTupleSerialize.hxx>
#include <ROOT/RNTupleUtil.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleZip.hxx>
#include <ROOT/RPageAllocator.hxx>
#include <ROOT/RPageSinkBuf.hxx>
#include <ROOT/RPageStorage.hxx>

#include <mpi.h>

#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

/// The alignment for write boundaries between allocator and MPI processes.
/// Because it is used as the write buffer size on the allocator, it must be a
/// multiple of 4096 (see RNTupleWriteOptions).
static constexpr std::size_t kAggregatorWriteAlignment = 4096;
static constexpr std::size_t kProcessWriteBufferSize = 4 * 1024 * 1024;

/// Tags for MPI communication with the aggregator.
static constexpr int kTagAggregator = 1;
static constexpr int kTagOffset = 2;

namespace {

using ROOT::Experimental::ClusterSize_t;
using ROOT::Experimental::DescriptorId_t;
using ROOT::Experimental::NTupleSize_t;
using ROOT::Experimental::RClusterDescriptor;
using ROOT::Experimental::RException;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleLocator;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleWriteOptions;
using ROOT::Experimental::Detail::RNTupleAtomicTimer;
using ROOT::Experimental::Detail::RNTupleMetrics;
using ROOT::Experimental::Internal::GetFieldZeroOfModel;
using ROOT::Experimental::Internal::GetProjectedFieldsOfModel;
using ROOT::Experimental::Internal::RClusterDescriptorBuilder;
using ROOT::Experimental::Internal::RClusterGroupDescriptorBuilder;
using ROOT::Experimental::Internal::RColumn;
using ROOT::Experimental::Internal::RColumnDescriptorBuilder;
using ROOT::Experimental::Internal::RFieldDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleCompressor;
using ROOT::Experimental::Internal::RNTupleDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleFileWriter;
using ROOT::Experimental::Internal::RNTupleModelChangeset;
using ROOT::Experimental::Internal::RNTupleSerializer;
using ROOT::Experimental::Internal::RPage;
using ROOT::Experimental::Internal::RPagePersistentSink;
using ROOT::Experimental::Internal::RPageSink;
using ROOT::Experimental::Internal::RPageStorage;

/// A persistent page sink based on RPageSinkFile used by the
/// RNTupleWriterMPIAggregator class.
class RPageSinkMPIAggregator final : public RPagePersistentSink {
  /// The file writer to write the ntuple.
  std::unique_ptr<RNTupleFileWriter> fWriter;
  /// The last offset in the file that was reserved in CommitSealedPageVImpl.
  std::uint64_t fLastOffset = 0;
  /// Whether the processes are expected to send the payload data.
  bool fProcessesSendData;

public:
  RPageSinkMPIAggregator(const RNTupleWriterMPI::Config &config)
      : RPagePersistentSink(config.fNTupleName, config.fOptions),
        fProcessesSendData(config.fSendData) {
    fCompressor = std::make_unique<RNTupleCompressor>();
    EnableDefaultMetrics("Aggregator");
    // No support for merging pages at the moment
    fFeatures.fCanMergePages = false;

    // Create the file writer, but force the write buffer size to avoid
    // overlapping writes on aggregator and processes.
    // TODO: This is pessimistic for writing the header and footer...
    RNTupleWriteOptions options = config.fOptions;
    if (!fProcessesSendData) {
      options.SetWriteBufferSize(kAggregatorWriteAlignment);
    }

    fWriter = RNTupleFileWriter::Recreate(
        config.fNTupleName, config.fStorage,
        RNTupleFileWriter::EContainerFormat::kTFile, options);
  }

  std::uint64_t GetLastOffset() const { return fLastOffset; }

  void InitImpl(unsigned char *serializedHeader,
                std::uint32_t length) override {
    // Copied from RPageSinkFile::InitImpl
    auto zipBuffer = std::make_unique<unsigned char[]>(length);
    auto szZipHeader = fCompressor->Zip(
        serializedHeader, length, GetWriteOptions().GetCompression(),
        RNTupleCompressor::MakeMemCopyWriter(zipBuffer.get()));
    fWriter->WriteNTupleHeader(zipBuffer.get(), szZipHeader, length);
  };

  RNTupleLocator CommitPageImpl(ColumnHandle_t, const RPage &) override {
    throw RException(R__FAIL(
        "should never commit a single page via RPageSinkMPIAggregator"));
    return {};
  }
  RNTupleLocator CommitSealedPageImpl(DescriptorId_t,
                                      const RSealedPage &) override {
    throw RException(R__FAIL(
        "should never commit a single page via RPageSinkMPIAggregator"));
    return {};
  }

  std::vector<RNTupleLocator>
  CommitSealedPageVImpl(std::span<RPageStorage::RSealedPageGroup> ranges,
                        const std::vector<bool> &mask) override {
    std::uint64_t sumSealedPages = 0, sumBytesPacked = 0;

    std::size_t nPages = 0;
    for (auto rangeIt = ranges.begin(); rangeIt != ranges.end(); ++rangeIt) {
      auto &range = *rangeIt;
      if (range.fFirst == range.fLast) {
        // Skip empty ranges, they might not have a physical column ID!
        continue;
      }

      const auto bitsOnStorage =
          fDescriptorBuilder.GetDescriptor()
              .GetColumnDescriptor(range.fPhysicalColumnId)
              .GetBitsOnStorage();

      for (auto sealedPageIt = range.fFirst; sealedPageIt != range.fLast;
           ++sealedPageIt) {
        assert(mask[nPages]);
        nPages++;

        const auto bytesPacked =
            (bitsOnStorage * sealedPageIt->GetNElements() + 7) / 8;
        sumSealedPages += sealedPageIt->GetBufferSize();
        sumBytesPacked += bytesPacked;
      }
    }

    assert(sumSealedPages < fOptions->GetMaxKeySize());
    assert(sumBytesPacked < fOptions->GetMaxKeySize());

    std::vector<RNTupleLocator> locators;
    {
      RNTupleAtomicTimer timer(fCounters->fTimeWallWrite,
                               fCounters->fTimeCpuWrite);

      std::uint64_t padding = 0;
      if (!fProcessesSendData) {
        // If the processes write the data directly, we need to pad the reserved
        // buffer accordingly to avoid overlapping writes. For the returned
        // offset, the worst case is one trailing byte.
        padding = kAggregatorWriteAlignment - 1;
        // For the end of the buffer, we just need to make sure there are enough
        // bytes behind the last sealed page to reach the next buffer.
        if (sumSealedPages % kAggregatorWriteAlignment != 0) {
          padding += kAggregatorWriteAlignment -
                     sumSealedPages % kAggregatorWriteAlignment;
        }
      }
      std::uint64_t offset =
          fWriter->ReserveBlob(sumSealedPages + padding, sumBytesPacked);
      if (!fProcessesSendData && offset % kAggregatorWriteAlignment != 0) {
        offset +=
            kAggregatorWriteAlignment - offset % kAggregatorWriteAlignment;
      }
      fLastOffset = offset;

      locators.reserve(nPages);

      for (auto rangeIt = ranges.begin(); rangeIt != ranges.end(); ++rangeIt) {
        for (auto sealedPageIt = rangeIt->fFirst;
             sealedPageIt != rangeIt->fLast; ++sealedPageIt) {
          const auto &sealedPage = *sealedPageIt;
          if (sealedPage.GetBuffer()) {
            assert(fProcessesSendData);
            // If the buffer is nullptr, the process did not send the data and
            // will instead write into this offset.
            fWriter->WriteIntoReservedBlob(sealedPage.GetBuffer(),
                                           sealedPage.GetBufferSize(), offset);
          } else {
            assert(!fProcessesSendData);
          }
          RNTupleLocator locator;
          locator.fPosition = offset;
          locator.fBytesOnStorage = sealedPage.GetDataSize();
          locators.push_back(locator);
          offset += sealedPage.GetBufferSize();
        }
      }
    }

    fCounters->fNPageCommitted.Add(nPages);
    fCounters->fSzWritePayload.Add(sumSealedPages);

    return locators;
  }

  std::uint64_t StageClusterImpl() override {
    // We don't care about the number of bytes written on the aggregator.
    return 0;
  }
  RNTupleLocator CommitClusterGroupImpl(unsigned char *serializedPageList,
                                        std::uint32_t length) override {
    // Copied from RPageSinkFile::CommitClusterGroupImpl
    auto bufPageListZip = std::make_unique<unsigned char[]>(length);
    auto szPageListZip = fCompressor->Zip(
        serializedPageList, length, GetWriteOptions().GetCompression(),
        RNTupleCompressor::MakeMemCopyWriter(bufPageListZip.get()));

    RNTupleLocator result;
    result.fBytesOnStorage = szPageListZip;
    result.fPosition =
        fWriter->WriteBlob(bufPageListZip.get(), szPageListZip, length);
    return result;
  }
  void CommitDatasetImpl(unsigned char *serializedFooter,
                         std::uint32_t length) override {
    // Copied from RPageSinkFile::CommitDatasetImpl
    fWriter->UpdateStreamerInfos(fDescriptorBuilder.BuildStreamerInfos());
    auto bufFooterZip = std::make_unique<unsigned char[]>(length);
    auto szFooterZip = fCompressor->Zip(
        serializedFooter, length, GetWriteOptions().GetCompression(),
        RNTupleCompressor::MakeMemCopyWriter(bufFooterZip.get()));
    fWriter->WriteNTupleFooter(bufFooterZip.get(), szFooterZip, length);
    fWriter->Commit();
  }
};

/// A helper class to aggregate the RNTuple data written collectively from
/// multiple processes.
class RNTupleWriterMPIAggregator {
  /// The persistent sink to write the ntuple.
  std::unique_ptr<RPageSinkMPIAggregator> fSink;
  /// The model to write the ntuple; needs to be destructed before fSink.
  std::unique_ptr<RNTupleModel> fModel;

  /// The MPI communicator handle.
  MPI_Comm fComm;

  /// Whether the processes are expected to send the payload data.
  bool fProcessesSendData;

public:
  RNTupleWriterMPIAggregator(const RNTupleWriterMPI::Config &config,
                             MPI_Comm comm)
      : fModel(config.fModel->Clone()), fComm(comm),
        fProcessesSendData(config.fSendData) {
    fSink = std::make_unique<RPageSinkMPIAggregator>(config);
    fModel->Freeze();
    fSink->Init(*fModel);
  }

  ~RNTupleWriterMPIAggregator() {
    // Commit the ntuple to storage.
    fSink->CommitClusterGroup();
    fSink->CommitDataset();
  }

  RNTupleMetrics &GetMetrics() { return fSink->GetMetrics(); }

  void Collect(int processes) {
    while (processes > 0) {
      MPI_Status status;
      // Do not use MPI_Mprobe, at least openmpi-4.1.1-5.el8.x86_64 seems to
      // have a bug where the pattern could deadlock if switching to MPI_Ssend!
      MPI_Probe(MPI_ANY_SOURCE, kTagAggregator, fComm, &status);

      int count;
      MPI_Get_count(&status, MPI_BYTE, &count);
      int source = status.MPI_SOURCE;

      std::unique_ptr<unsigned char[]> buf;
      if (count > 0) {
        buf = std::make_unique<unsigned char[]>(count);
      }

      MPI_Recv(buf.get(), count, MPI_BYTE, source, kTagAggregator, fComm,
               MPI_STATUS_IGNORE);

      if (count == 0) {
        // The process signaled it is done. Send back an empty message.
        processes--;
        continue;
      }

      // The process wants to commit a cluster. Create a temporary descriptor
      // to deserialize the process' page list.
      RNTupleDescriptorBuilder descriptorBuilder;
      descriptorBuilder.SetNTuple("ntuple", "");
      RClusterGroupDescriptorBuilder cgBuilder;
      cgBuilder.ClusterGroupId(0).NClusters(1);
      descriptorBuilder.AddClusterGroup(cgBuilder.MoveDescriptor().Unwrap());
      auto descriptor = descriptorBuilder.MoveDescriptor();
      RNTupleSerializer::DeserializePageList(buf.get(), count, 0, descriptor);
      auto &clusterDescriptor = descriptor.GetClusterDescriptor(0);

      unsigned char *ptr = nullptr;
      if (fProcessesSendData) {
        // The first 64 bits of the envelope is the type and the length.
        std::uint64_t typeAndSize;
        RNTupleSerializer::DeserializeUInt64(buf.get(), typeAndSize);
        std::uint64_t envelopeSize = typeAndSize >> 16;

        ptr = buf.get() + envelopeSize;
      }

      // Rebuild the list of sealed pages. If the process sent the data, they
      // will point into the message buffer. Otherwise, the buffer will be the
      // nullptr and aggregator will not write them.
      std::deque<RPageStorage::SealedPageSequence_t> sealedPagesV;
      std::vector<RPageStorage::RSealedPageGroup> sealedPageGroups;
      auto nColumns = clusterDescriptor.GetColumnRangeIterable().size();
      DescriptorId_t columnId = 0;
      while (nColumns > 0) {
        if (!clusterDescriptor.ContainsColumn(columnId)) {
          columnId++;
          continue;
        }

        nColumns--;

        auto &columnRange = clusterDescriptor.GetColumnRange(columnId);
        if (columnRange.fIsSuppressed) {
          // TODO: Mark as suppressed
          columnId++;
          continue;
        }

        auto &pageRange = clusterDescriptor.GetPageRange(columnId);
        RPageStorage::SealedPageSequence_t sealedPages;
        for (const auto &pageInfo : pageRange.fPageInfos) {
          auto bytesOnStorage = pageInfo.fLocator.fBytesOnStorage;
          sealedPages.emplace_back(ptr, bytesOnStorage, pageInfo.fNElements,
                                   pageInfo.fHasChecksum);
          if (ptr) {
            ptr += bytesOnStorage;
          }
        }

        sealedPagesV.push_back(std::move(sealedPages));
        sealedPageGroups.emplace_back(columnId, sealedPagesV.back().cbegin(),
                                      sealedPagesV.back().cend());

        columnId++;
      }
      assert(ptr == nullptr || ptr == buf.get() + count);

      fSink->CommitSealedPageV(sealedPageGroups);
      fSink->CommitCluster(clusterDescriptor.GetNEntries());

      // Send the response.
      int size = 0;
      unsigned char bufOffset[sizeof(std::uint64_t)];
      if (!fProcessesSendData) {
        std::uint64_t offset = fSink->GetLastOffset();
        RNTupleSerializer::SerializeUInt64(offset, &bufOffset);
        size = sizeof(std::uint64_t);
      }
      MPI_Send(&bufOffset, size, MPI_BYTE, source, kTagOffset, fComm);
    }
  }
};

/// A page sink that writes RNTuple data collectively from multiple processes
/// using MPI.
class RPageSinkMPI final : public RPageSink {
  /// A helper struct to keep information about a column and buffer all sealed
  /// pages that were committed to this page sink.
  struct RColumnBuf {
    struct RPageBuf {
      RSealedPage fSealedPage;
      std::unique_ptr<unsigned char[]> fBuffer;
    };

    std::vector<RPageBuf> fPages;
    ClusterSize_t fNElements{0};
    bool fIsSuppressed = false;
  };

  /// A list of all known columns, indexed by their physical id.
  std::vector<RColumnBuf> fBufferedColumns;

  /// A dummy serialization context to send information about a cluster to the
  /// aggregator; see CommitCluster.
  RNTupleSerializer::RContext fSerializationContext;
  /// A dummy descriptor builder to send information about a cluster to the
  /// aggregator; see CommitCluster.
  RNTupleDescriptorBuilder fDescriptorBuilder;

  /// The MPI communicator handle.
  MPI_Comm fComm;
  /// The root process rank running the aggregator.
  int fRoot;

  /// The background thread to run the aggregator on the root.
  std::thread fAggregatorThread;
  /// The aggregator running on the root.
  std::unique_ptr<RNTupleWriterMPIAggregator> fAggregator;

  /// Storage path for opening the same file on all processes.
  std::string fStorage;
  /// The opened file descriptor.
  int fFileDes = -1;
  /// A scratch area to buffer writes to the file.
  unsigned char *fBlock = nullptr;
  /// Whether to send the payload data via MPI.
  bool fSendData;

public:
  RPageSinkMPI(const RNTupleWriterMPI::Config &config, int root, MPI_Comm comm)
      : RPageSink(config.fNTupleName, config.fOptions), fRoot(root),
        fStorage(config.fStorage), fSendData(config.fSendData) {
    fMetrics = RNTupleMetrics("RPageSinkMPI");
    MPI_Comm_dup(comm, &fComm);

    if (!fSendData) {
      std::align_val_t blockAlign{kAggregatorWriteAlignment};
      fBlock = static_cast<unsigned char *>(
          ::operator new[](kProcessWriteBufferSize, blockAlign));
      memset(fBlock, 0, kProcessWriteBufferSize);
    }

    int rank, size;
    MPI_Comm_rank(fComm, &rank);
    MPI_Comm_size(fComm, &size);

    if (rank == root) {
      fAggregator = std::make_unique<RNTupleWriterMPIAggregator>(config, fComm);
      fMetrics.ObserveMetrics(fAggregator->GetMetrics());
      fAggregatorThread =
          std::thread([this, size]() { fAggregator->Collect(size); });
    }
  }

  ~RPageSinkMPI() final {
    if (fFileDes > 0) {
      close(fFileDes);
    }
    if (fBlock) {
      std::align_val_t blockAlign{kAggregatorWriteAlignment};
      ::operator delete[](fBlock, blockAlign);
    }

    if (fAggregator) {
      fAggregatorThread.join();
      fAggregator.reset();
    }

    MPI_Comm_free(&fComm);
  }

  const RNTupleDescriptor &GetDescriptor() const final {
    static RNTupleDescriptor descriptor;
    return descriptor;
  }

  ColumnHandle_t AddColumn(DescriptorId_t fieldId, RColumn &column) final {
    auto columnId = fDescriptorBuilder.GetDescriptor().GetNPhysicalColumns();
    RColumnDescriptorBuilder columnBuilder;
    columnBuilder.LogicalColumnId(columnId)
        .PhysicalColumnId(columnId)
        .FieldId(fieldId)
        .BitsOnStorage(column.GetBitsOnStorage())
        .Type(column.GetType())
        .Index(column.GetIndex())
        .RepresentationIndex(column.GetRepresentationIndex())
        .FirstElementIndex(column.GetFirstElementIndex());
    fDescriptorBuilder.AddColumn(columnBuilder.MakeDescriptor().Unwrap());
    return {columnId, &column};
  }

  void InitImpl(RNTupleModel &model) final {
    auto &fieldZero = GetFieldZeroOfModel(model);
    fDescriptorBuilder.AddField(RFieldDescriptorBuilder::FromField(fieldZero)
                                    .FieldId(0)
                                    .MakeDescriptor()
                                    .Unwrap());
    fieldZero.SetOnDiskId(0);
    auto &projectedFields = GetProjectedFieldsOfModel(model);
    projectedFields.GetFieldZero().SetOnDiskId(0);

    const auto &descriptor = fDescriptorBuilder.GetDescriptor();
    for (auto &f : fieldZero) {
      auto fieldId = descriptor.GetNFields();
      fDescriptorBuilder.AddField(RFieldDescriptorBuilder::FromField(f)
                                      .FieldId(fieldId)
                                      .MakeDescriptor()
                                      .Unwrap());
      fDescriptorBuilder.AddFieldLink(f.GetParent()->GetOnDiskId(), fieldId);
      f.SetOnDiskId(fieldId);
      CallConnectPageSinkOnField(f, *this);
    }

    fBufferedColumns.resize(
        fDescriptorBuilder.GetDescriptor().GetNPhysicalColumns());

    // Prepare the serialization context. We could serialize the entire header,
    // but we don't need it.
    fSerializationContext.MapSchema(descriptor, /*forHeaderExtension=*/false);
  }
  void UpdateSchema(const RNTupleModelChangeset &, NTupleSize_t) final {
    throw RException(R__FAIL("UpdateSchema not supported via RPageSinkMPI"));
  }
  void UpdateExtraTypeInfo(const RExtraTypeInfoDescriptor &) final {
    throw RException(
        R__FAIL("UpdateExtraTypeInfo not supported via RPageSinkMPI"));
  }

  void CommitSuppressedColumn(ColumnHandle_t columnHandle) final {
    fBufferedColumns.at(columnHandle.fPhysicalId).fIsSuppressed = true;
  }
  void CommitPage(ColumnHandle_t columnHandle, const RPage &page) final {
    auto &pageBuf =
        fBufferedColumns.at(columnHandle.fPhysicalId).fPages.emplace_back();
    auto bufferSize =
        page.GetNBytes() +
        GetWriteOptions().GetEnablePageChecksums() * kNBytesPageChecksum;
    pageBuf.fBuffer =
        std::unique_ptr<unsigned char[]>(new unsigned char[bufferSize]);

    RSealPageConfig config;
    config.fPage = &page;
    config.fElement = columnHandle.fColumn->GetElement();
    config.fCompressionSetting = GetWriteOptions().GetCompression();
    config.fWriteChecksum = GetWriteOptions().GetEnablePageChecksums();
    config.fAllowAlias = false;
    config.fBuffer = pageBuf.fBuffer.get();
    pageBuf.fSealedPage = SealPage(config);
  }
  void CommitSealedPage(DescriptorId_t, const RSealedPage &) final {
    throw RException(
        R__FAIL("should never commit a single sealed page via RPageSinkMPI"));
  }
  void
  CommitSealedPageV(std::span<RPageStorage::RSealedPageGroup> ranges) final {
    for (auto &range : ranges) {
      auto &columnBuf = fBufferedColumns.at(range.fPhysicalColumnId);
      for (auto sealedPageIt = range.fFirst; sealedPageIt != range.fLast;
           ++sealedPageIt) {
        columnBuf.fNElements += sealedPageIt->GetNElements();

        auto &pageBuf = columnBuf.fPages.emplace_back();
        pageBuf.fSealedPage = *sealedPageIt;
        // Copy the sealed page buffer; TODO: maybe this is not needed with
        // RPageSinkBuf?
        auto bufferSize = sealedPageIt->GetBufferSize();
        pageBuf.fBuffer =
            std::unique_ptr<unsigned char[]>(new unsigned char[bufferSize]);
        memcpy(pageBuf.fBuffer.get(), sealedPageIt->GetBuffer(), bufferSize);
        pageBuf.fSealedPage.SetBuffer(pageBuf.fBuffer.get());
      }
    }
  }

  RStagedCluster StageCluster(NTupleSize_t) final {
    throw RException(
        R__FAIL("staged cluster committing not supported via RPageSinkMPI"));
  }
  void CommitStagedClusters(std::span<RStagedCluster>) final {
    throw RException(
        R__FAIL("staged cluster committing not supported via RPageSinkMPI"));
  }

  std::uint64_t CommitCluster(NTupleSize_t nNewEntries) final {
    const auto &descriptor = fDescriptorBuilder.GetDescriptor();

    // Build a RClusterDescriptor based on the buffered columns and pages.
    std::uint64_t sumSealedPages = 0;
    RClusterDescriptorBuilder clusterBuilder;
    DescriptorId_t clusterId = descriptor.GetNActiveClusters();
    // First entry index are left unset.
    clusterBuilder.ClusterId(clusterId).NEntries(nNewEntries);
    for (unsigned int i = 0; i < fBufferedColumns.size(); ++i) {
      // Assert that ids are the same in memory and on disk. Otherwise we would
      // need to reorder column pages.
      assert(fSerializationContext.GetOnDiskColumnId(i) == i);
      auto &columnBuf = fBufferedColumns[i];
      if (columnBuf.fIsSuppressed) {
        assert(columnBuf.fPages.empty());
        clusterBuilder.MarkSuppressedColumnRange(i);
      } else {
        RClusterDescriptor::RPageRange pageRange;
        for (auto &pageBuf : columnBuf.fPages) {
          RClusterDescriptor::RPageRange::RPageInfo pageInfo;
          pageInfo.fNElements = pageBuf.fSealedPage.GetNElements();
          // For the locator, we only set the (compressed) size.
          auto sealedBufferSize = pageBuf.fSealedPage.GetBufferSize();
          pageInfo.fLocator.fBytesOnStorage = sealedBufferSize;
          pageInfo.fHasChecksum = pageBuf.fSealedPage.GetHasChecksum();
          sumSealedPages += sealedBufferSize;
          pageRange.fPageInfos.emplace_back(pageInfo);
        }
        pageRange.fPhysicalColumnId = i;
        // First element index is left unset.
        int firstElementIndex = 0;
        int compressionSettings = GetWriteOptions().GetCompression();
        clusterBuilder.CommitColumnRange(i, firstElementIndex,
                                         compressionSettings, pageRange);
      }
    }
    fDescriptorBuilder.AddCluster(clusterBuilder.MoveDescriptor().Unwrap());

    // Serialize the RClusterDescriptor and send via the socket.
    DescriptorId_t physClusterIDs[] = {
        fSerializationContext.MapClusterId(clusterId)};
    auto szPageList = RNTupleSerializer::SerializePageList(
        nullptr, descriptor, physClusterIDs, fSerializationContext);

    auto szBuffer = szPageList;
    if (fSendData) {
      szBuffer += sumSealedPages;
    }
    auto buffer = std::make_unique<unsigned char[]>(szBuffer);
    RNTupleSerializer::SerializePageList(buffer.get(), descriptor,
                                         physClusterIDs, fSerializationContext);

    if (fSendData) {
      // Append all sealed page buffers.
      unsigned char *ptr = buffer.get() + szPageList;
      for (auto &columnBuf : fBufferedColumns) {
        for (auto &pageBuf : columnBuf.fPages) {
          auto sealedBufferSize = pageBuf.fSealedPage.GetBufferSize();
          memcpy(ptr, pageBuf.fSealedPage.GetBuffer(), sealedBufferSize);
          ptr += sealedBufferSize;
        }
      }
      assert(ptr == buffer.get() + szBuffer);
    }

    MPI_Send(buffer.get(), szBuffer, MPI_BYTE, fRoot, kTagAggregator, fComm);

    // Get back the reply, the message is empty unless we did not send the data.
    unsigned char bufOffset[sizeof(std::uint64_t)];
    MPI_Status status;
    MPI_Recv(&bufOffset, sizeof(bufOffset), MPI_BYTE, fRoot, kTagOffset, fComm,
             &status);

    if (!fSendData) {
      // After the aggregator replied, we are guaranteed that it created the
      // file and we can open it for parallel writing.
      if (fFileDes < 0) {
        int flags = O_WRONLY;
#ifdef O_LARGEFILE
        // Add the equivalent flag that is passed by fopen64.
        flags |= O_LARGEFILE;
#endif
        if (fOptions->GetUseDirectIO()) {
          flags |= O_DIRECT;
        }
        fFileDes = open(fStorage.c_str(), flags, 0666);
        if (fFileDes < 0) {
          throw RException(R__FAIL("open() failed"));
        }
      }

#ifndef NDEBUG
      int count;
      MPI_Get_count(&status, MPI_BYTE, &count);
      assert(count == sizeof(std::uint64_t));
#endif
      std::uint64_t offset;
      RNTupleSerializer::DeserializeUInt64(&bufOffset, offset);
      assert(offset % kAggregatorWriteAlignment == 0);
      std::uint64_t blockOffset = offset;

      // Write the sealed page buffers in the same order.
      for (auto &columnBuf : fBufferedColumns) {
        for (auto &pageBuf : columnBuf.fPages) {
          auto nBytesPage = pageBuf.fSealedPage.GetBufferSize();
          const unsigned char *buffer = static_cast<const unsigned char *>(
              pageBuf.fSealedPage.GetBuffer());
          while (nBytesPage > 0) {
            std::uint64_t posInBlock = offset - blockOffset;
            if (posInBlock >= kProcessWriteBufferSize) {
              // Write the block.
              std::size_t retval = pwrite(fFileDes, fBlock,
                                          kProcessWriteBufferSize, blockOffset);
              if (retval != kProcessWriteBufferSize)
                throw RException(
                    R__FAIL(std::string("write failed: ") + strerror(errno)));

              // Null the buffer contents for good measure.
              memset(fBlock, 0, kProcessWriteBufferSize);
              posInBlock = offset % kAggregatorWriteAlignment;
              blockOffset = offset - posInBlock;
            }

            std::size_t blockSize = nBytesPage;
            if (blockSize > kProcessWriteBufferSize - posInBlock) {
              blockSize = kProcessWriteBufferSize - posInBlock;
            }
            memcpy(fBlock + posInBlock, buffer, blockSize);
            buffer += blockSize;
            nBytesPage -= blockSize;
            offset += blockSize;
          }
        }
      }

      // Flush the buffer if any data left.
      if (offset > blockOffset) {
        std::size_t lastBlockSize = offset - blockOffset;
        // Round up to a multiple of kAggregatorWriteAlignment.
        lastBlockSize += kAggregatorWriteAlignment - 1;
        lastBlockSize = (lastBlockSize / kAggregatorWriteAlignment) *
                        kAggregatorWriteAlignment;
        std::size_t retval =
            pwrite(fFileDes, fBlock, lastBlockSize, blockOffset);
        if (retval != lastBlockSize)
          throw RException(
              R__FAIL(std::string("write failed: ") + strerror(errno)));

        // Null the buffer contents for good measure.
        memset(fBlock, 0, kProcessWriteBufferSize);
      }
    }

    // Clean up all buffered columns and pages.
    for (auto &columnBuf : fBufferedColumns) {
      columnBuf.fPages.clear();
      columnBuf.fNElements = 0;
      columnBuf.fIsSuppressed = false;
    }

    return sumSealedPages;
  }
  void CommitClusterGroup() final {
    // TODO or ignored?
  }
  void CommitDatasetImpl() final {
    MPI_Send(NULL, 0, MPI_BYTE, fRoot, kTagAggregator, fComm);
  }
};

} // namespace

std::unique_ptr<ROOT::Experimental::RNTupleWriter>
RNTupleWriterMPI::Recreate(Config config, int root, MPI_Comm comm) {
  int flag = 0;
  MPI_Initialized(&flag);
  if (!flag) {
    throw RException(R__FAIL("MPI library was not initialized"));
  }
  int provided = -1;
  MPI_Query_thread(&provided);
  if (provided != MPI_THREAD_MULTIPLE) {
    throw RException(R__FAIL("RNTupleWriterMPI requires MPI_THREAD_MULTIPLE"));
  }

  std::unique_ptr<ROOT::Experimental::Internal::RPageSink> sink =
      std::make_unique<RPageSinkMPI>(config, root, comm);
  if (config.fOptions.GetUseBufferedWrite()) {
    sink = std::make_unique<ROOT::Experimental::Internal::RPageSinkBuf>(
        std::move(sink));
  }

  return ROOT::Experimental::Internal::CreateRNTupleWriter(
      std::move(config.fModel), std::move(sink));
}
