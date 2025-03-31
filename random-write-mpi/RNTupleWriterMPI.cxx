// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterMPI.hxx"

#include <ROOT/RError.hxx>
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

#include <condition_variable>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

static constexpr std::size_t kProcessWriteBufferSize = 4 * 1024 * 1024;

/// Tags for MPI communication with the aggregator.
static constexpr int kTagAggregator = 1;
static constexpr int kTagOffset = 2;

/// Offset in the file to store the global offset (at the beginning by default).
static constexpr off_t kGlobalOffsetOff = 0;
using GlobalOffsetType = std::uint64_t;
static constexpr MPI_Datatype kGlobalOffsetDatatype = MPI_UINT64_T;

namespace {

using ROOT::DescriptorId_t;
using ROOT::NTupleSize_t;
using ROOT::RNTupleLocator;
using ROOT::Experimental::RClusterDescriptor;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::Detail::RNTupleAtomicCounter;
using ROOT::Experimental::Detail::RNTupleAtomicTimer;
using ROOT::Experimental::Detail::RNTupleMetrics;
using ROOT::Experimental::Detail::RNTupleTickCounter;
using ROOT::Experimental::Internal::GetFieldZeroOfModel;
using ROOT::Experimental::Internal::GetProjectedFieldsOfModel;
using ROOT::Experimental::Internal::RClusterDescriptorBuilder;
using ROOT::Experimental::Internal::RClusterGroupDescriptorBuilder;
using ROOT::Experimental::Internal::RColumn;
using ROOT::Experimental::Internal::RColumnDescriptorBuilder;
using ROOT::Experimental::Internal::RFieldDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleFileWriter;
static constexpr auto kBlobKeyLen =
    ROOT::Experimental::Internal::RNTupleFileWriter::kBlobKeyLen;
using ROOT::Experimental::Internal::RNTupleModelChangeset;
using ROOT::Experimental::Internal::RNTupleSerializer;
using ROOT::Experimental::Internal::RPage;
using ROOT::Experimental::Internal::RPagePersistentSink;
using ROOT::Experimental::Internal::RPageSink;
using ROOT::Experimental::Internal::RPageStorage;
using ROOT::Internal::RNTupleCompressor;

/// A persistent page sink based on RPageSinkFile used by the
/// RNTupleWriterMPIAggregator class.
class RPageSinkMPIAggregator final : public RPagePersistentSink {
  /// The file writer to write the ntuple.
  std::unique_ptr<RNTupleFileWriter> fWriter;
  /// The last offset in the file that was reserved in CommitSealedPageVImpl.
  std::uint64_t fLastOffset = 0;
  /// The current offset in the file.
  std::uint64_t fCurrentOffset = 0;
  /// The key buffer when sending to the processes.
  unsigned char fKeyBuffer[kBlobKeyLen];
  /// The alignment for write boundaries between processes.
  std::size_t fWriteAlignment;
  /// Whether the processes are expected to send the payload data.
  bool fProcessesSendData;
  /// Whether to send the key to the processes instead of writing.
  bool fSendKey;

public:
  RPageSinkMPIAggregator(const RNTupleWriterMPI::Config &config)
      : RPagePersistentSink(config.fNTupleName, config.fOptions),
        fWriteAlignment(config.fWriteAlignment),
        fProcessesSendData(config.fSendData), fSendKey(config.fSendKey) {
    EnableDefaultMetrics("Aggregator");
    // No support for merging pages at the moment
    fFeatures.fCanMergePages = false;

    // Create the file writer, but force the write buffer size to avoid
    // overlapping writes on aggregator and processes.
    // TODO: This is pessimistic for writing the header and footer...
    ROOT::RNTupleWriteOptions options = config.fOptions;
    if (!fProcessesSendData) {
      options.SetWriteBufferSize(fWriteAlignment);
    }

    fWriter = RNTupleFileWriter::Recreate(
        config.fNTupleName, config.fStorage,
        RNTupleFileWriter::EContainerFormat::kTFile, options);
  }

  std::uint64_t GetLastOffset() const { return fLastOffset; }
  std::uint64_t GetCurrentOffset() const { return fCurrentOffset; }
  const unsigned char *GetKeyBuffer() const { return &fKeyBuffer[0]; }

  void Seek(std::uint64_t offset) {
    fCurrentOffset = offset;
    fWriter->Seek(offset);
  }

  /// Append already written cluster, copying the column and page ranges.
  void AppendCluster(const RClusterDescriptor &clusterDescriptor) {
    RStagedCluster stagedClusters[1];
    RStagedCluster &stagedCluster = stagedClusters[0];
    stagedCluster.fNEntries = clusterDescriptor.GetNEntries();

    std::size_t nPages = 0;
    std::uint64_t nBytesWritten = 0;
    for (const auto &columnRange : clusterDescriptor.GetColumnRangeIterable()) {
      const auto &pageRange =
          clusterDescriptor.GetPageRange(columnRange.GetPhysicalColumnId());
      nPages += pageRange.GetPageInfos().size();
      for (const auto &pageInfo : pageRange.GetPageInfos()) {
        nBytesWritten += pageInfo.GetLocator().GetNBytesOnStorage();
      }

      // Just copy all members into a column info.
      RStagedCluster::RColumnInfo columnInfo;
      columnInfo.fPageRange = pageRange.Clone();
      columnInfo.fNElements = columnRange.GetNElements();
      columnInfo.fCompressionSettings =
          columnRange.GetCompressionSettings().value();
      columnInfo.fIsSuppressed = columnRange.IsSuppressed();
      stagedCluster.fColumnInfos.push_back(std::move(columnInfo));
    }

    stagedCluster.fNBytesWritten = nBytesWritten;

    CommitStagedClusters(stagedClusters);

    fCounters->fNPageCommitted.Add(nPages);
    fCounters->fSzWritePayload.Add(nBytesWritten);
  }

  void InitImpl(unsigned char *serializedHeader,
                std::uint32_t length) override {
    // Copied from RPageSinkFile::InitImpl
    std::unique_ptr<unsigned char[]> zipBuffer(new unsigned char[length]);
    auto szZipHeader = RNTupleCompressor::Zip(
        serializedHeader, length, GetWriteOptions().GetCompression(),
        zipBuffer.get());
    auto offset =
        fWriter->WriteNTupleHeader(zipBuffer.get(), szZipHeader, length);
    fCurrentOffset = offset + szZipHeader;
    if (!fProcessesSendData && fCurrentOffset % fWriteAlignment != 0) {
      // Insert a dummy blob to make the offset aligned. For this, we need at
      // least kBlobKeyLen bytes to write the key.
      auto dummyOffset = fCurrentOffset + kBlobKeyLen;
      std::size_t bytes = 0;
      if (dummyOffset % fWriteAlignment != 0) {
        bytes = fWriteAlignment - dummyOffset % fWriteAlignment;
      }
      offset = fWriter->ReserveBlob(bytes, 0);
      R__ASSERT(offset == dummyOffset);
      fCurrentOffset = offset + bytes;
    }
    R__ASSERT(fProcessesSendData || fCurrentOffset % fWriteAlignment == 0);
  };

  RNTupleLocator CommitPageImpl(ColumnHandle_t, const RPage &) override {
    throw ROOT::RException(R__FAIL(
        "should never commit a single page via RPageSinkMPIAggregator"));
    return {};
  }
  RNTupleLocator CommitSealedPageImpl(DescriptorId_t,
                                      const RSealedPage &) override {
    throw ROOT::RException(R__FAIL(
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
        // buffer accordingly to avoid overlapping writes.
        R__ASSERT(fCurrentOffset % fWriteAlignment == 0);
        if (!fSendKey) {
          // For the key header, we know that the current offset is aligned and
          // we need to pad until the next alignment boundary.
          padding = fWriteAlignment - kBlobKeyLen;
          // For the end of the buffer, we again need to pad until the next
          // alignment boundary.
          if (sumSealedPages % fWriteAlignment != 0) {
            padding += fWriteAlignment - sumSealedPages % fWriteAlignment;
          }
        } else {
          // If the processes also write the key, need to factor this in.
          auto totalSize = kBlobKeyLen + sumSealedPages;
          // Then we need to pad until the next alignment boundary.
          if (totalSize % fWriteAlignment != 0) {
            padding += fWriteAlignment - totalSize % fWriteAlignment;
          }
        }
      }
      unsigned char *keyBuffer = nullptr;
      if (fSendKey) {
        keyBuffer = fKeyBuffer;
      }
      std::uint64_t offset = fWriter->ReserveBlob(sumSealedPages + padding,
                                                  sumBytesPacked, keyBuffer);
      R__ASSERT(offset == fCurrentOffset + kBlobKeyLen);
      fCurrentOffset = offset + sumSealedPages + padding;
      R__ASSERT(fProcessesSendData || fCurrentOffset % fWriteAlignment == 0);

      if (!fProcessesSendData) {
        R__ASSERT(offset % fWriteAlignment == kBlobKeyLen);
        if (!fSendKey) {
          offset += fWriteAlignment - kBlobKeyLen;
        }
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
          locator.SetPosition(offset);
          locator.SetNBytesOnStorage(sealedPage.GetDataSize());
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
    std::unique_ptr<unsigned char[]> bufPageListZip(new unsigned char[length]);
    auto szPageListZip = RNTupleCompressor::Zip(
        serializedPageList, length, GetWriteOptions().GetCompression(),
        bufPageListZip.get());

    RNTupleLocator result;
    result.SetNBytesOnStorage(szPageListZip);
    result.SetPosition(
        fWriter->WriteBlob(bufPageListZip.get(), szPageListZip, length));
    return result;
  }
  void CommitDatasetImpl(unsigned char *serializedFooter,
                         std::uint32_t length) override {
    // Copied from RPageSinkFile::CommitDatasetImpl
    fWriter->UpdateStreamerInfos(fDescriptorBuilder.BuildStreamerInfos());
    std::unique_ptr<unsigned char[]> bufFooterZip(new unsigned char[length]);
    auto szFooterZip = RNTupleCompressor::Zip(
        serializedFooter, length, GetWriteOptions().GetCompression(),
        bufFooterZip.get());
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

  /// The root process rank running the aggregator.
  int fRoot;
  /// The MPI communicator handle.
  MPI_Comm fComm;

  /// A mutex to reduce contention in the MPI library on the root.
  std::mutex fMutex;
  /// A condition variable to signal from the aggregator.
  std::condition_variable fCV;
  /// The signal from the aggregator.
  bool fSignal = false;

  /// Whether the processes are expected to send the payload data.
  bool fProcessesSendData;
  /// Whether to send the key to the processes instead of writing.
  bool fSendKey;
  /// Whether to reduce contention in the MPI library on the root.
  bool fReduceRootContention;

public:
  RNTupleWriterMPIAggregator(const RNTupleWriterMPI::Config &config, int root,
                             MPI_Comm comm)
      : fModel(config.fModel->Clone()), fRoot(root), fComm(comm),
        fProcessesSendData(config.fSendData), fSendKey(config.fSendKey),
        fReduceRootContention(config.fReduceRootContention) {
    fSink = std::make_unique<RPageSinkMPIAggregator>(config);
    fModel->Freeze();
    fSink->Init(*fModel);
  }

  ~RNTupleWriterMPIAggregator() { CommitDataset(); }

  RNTupleMetrics &GetMetrics() { return fSink->GetMetrics(); }
  std::uint64_t GetCurrentOffset() const { return fSink->GetCurrentOffset(); }

  void Seek(std::uint64_t offset) { fSink->Seek(offset); }

  void AppendCluster(const RClusterDescriptor &clusterDescriptor) {
    fSink->AppendCluster(clusterDescriptor);
  }

  void WaitForSignal() {
    assert(fReduceRootContention);
    std::unique_lock lk(fMutex);
    fCV.wait(lk, [this] { return fSignal; });
    fSignal = false;
  }

private:
  void SignalFromAggregator() {
    assert(fReduceRootContention);
    {
      std::unique_lock lk(fMutex);
      fSignal = true;
    }
    fCV.notify_one();
  }

public:
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
        buf.reset(new unsigned char[count]);
      }

      MPI_Recv(buf.get(), count, MPI_BYTE, source, kTagAggregator, fComm,
               MPI_STATUS_IGNORE);

      if (count == 0) {
        // The process signaled it is done.
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
      RNTupleSerializer::DeserializePageList(
          buf.get(), count, 0, descriptor,
          RNTupleSerializer::EDescriptorDeserializeMode::kRaw);
      auto &clusterDescriptor = descriptor.GetClusterDescriptor(0);

      unsigned char *ptr = nullptr;
      if (fProcessesSendData) {
        // The first 64 bits of the envelope is the type and the length.
        std::uint64_t typeAndSize;
        RNTupleSerializer::DeserializeUInt64(buf.get(), typeAndSize);
        std::uint64_t envelopeSize = typeAndSize >> 16;

        ptr = buf.get() + envelopeSize;
      }
      std::uint64_t offset = 0;

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
        if (columnRange.IsSuppressed()) {
          // TODO: Mark as suppressed
          columnId++;
          continue;
        }

        auto &pageRange = clusterDescriptor.GetPageRange(columnId);
        RPageStorage::SealedPageSequence_t sealedPages;
        for (const auto &pageInfo : pageRange.GetPageInfos()) {
          const auto bufferSize =
              pageInfo.GetLocator().GetNBytesOnStorage() +
              pageInfo.HasChecksum() * RPageStorage::kNBytesPageChecksum;
          sealedPages.emplace_back(ptr, bufferSize, pageInfo.GetNElements(),
                                   pageInfo.HasChecksum());
          assert(pageInfo.GetLocator().GetPosition<std::uint64_t>() == offset);
          offset += bufferSize;
          if (ptr) {
            ptr += bufferSize;
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
      unsigned char sendBuf[sizeof(std::uint64_t) + kBlobKeyLen];
      if (!fProcessesSendData) {
        std::uint64_t offset = fSink->GetLastOffset();
        RNTupleSerializer::SerializeUInt64(offset, &sendBuf[0]);
        size = sizeof(std::uint64_t);
        if (fSendKey) {
          memcpy(&sendBuf[size], fSink->GetKeyBuffer(), kBlobKeyLen);
          size += kBlobKeyLen;
        }
      }
      MPI_Send(&sendBuf[0], size, MPI_BYTE, source, kTagOffset, fComm);

      if (fReduceRootContention && source == fRoot) {
        SignalFromAggregator();
      }
    }
  }

  void CommitDataset() {
    if (fModel->IsExpired())
      return;

    fSink->CommitClusterGroup();
    fSink->CommitDataset();
    fModel->Expire();
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
    NTupleSize_t fNElements{0};
    bool fIsSuppressed = false;
  };

  /// A list of all known columns, indexed by their physical id.
  std::vector<RColumnBuf> fBufferedColumns;

  /// The number of buffered pages.
  std::uint64_t fNPages = 0;
  /// The sum of all sealed pages.
  std::uint64_t fSumSealedPages = 0;
  /// The sum of packed bytes in all pages.
  std::uint64_t fSumBytesPacked = 0;

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
  /// The current rank in the communicator.
  int fRank;
  /// The size of the MPI communicator.
  int fSize;
  /// The global offset window.
  MPI_Win fOffsetWindow = MPI_WIN_NULL;
  /// The pointer to the global offset.
  std::uint64_t *fOffsetPtr = nullptr;

  /// The background thread to run the aggregator on the root.
  std::thread fAggregatorThread;
  /// The aggregator running on the root.
  std::unique_ptr<RNTupleWriterMPIAggregator> fAggregator;

  struct RCounters {
    RNTupleAtomicCounter &fNPageCommitted;
    RNTupleAtomicCounter &fSzWritePayload;
    RNTupleAtomicCounter &fTimeWallWrite;
    RNTupleAtomicCounter &fTimeWallCommAggregator;
    RNTupleAtomicCounter &fTimeWallGlobalOffset;
    RNTupleTickCounter<RNTupleAtomicCounter> &fTimeCpuWrite;
    RNTupleTickCounter<RNTupleAtomicCounter> &fTimeCpuCommAggregator;
    RNTupleTickCounter<RNTupleAtomicCounter> &fTimeCpuGlobalOffset;
  };
  std::unique_ptr<RCounters> fCounters;

  /// Storage path for opening the same file on all processes.
  std::string fStorage;
  /// The opened file descriptor.
  int fFileDes = -1;
  /// A scratch area to buffer writes to the file.
  unsigned char *fBlock = nullptr;
  /// The alignment for write boundaries between processes.
  std::size_t fWriteAlignment = 4096;
  /// Whether to send the payload data via MPI.
  bool fSendData;
  /// Whether the aggregator sends the key.
  bool fAggregatorSendsKey;
  /// Whether to reduce contention in the MPI library on the root.
  bool fReduceRootContention;
  /// Whether to write without aggregator, using a global offset.
  RNTupleWriterMPI::GlobalOffset fUseGlobalOffset = RNTupleWriterMPI::kFalse;
  /// The opened file descriptor to store the global offset (may alias
  /// fFileDes).
  int fGlobalOffsetFileDes = -1;

public:
  RPageSinkMPI(const RNTupleWriterMPI::Config &config, int root, MPI_Comm comm)
      : RPageSink(config.fNTupleName, config.fOptions), fRoot(root),
        fStorage(config.fStorage), fSendData(config.fSendData),
        fWriteAlignment(config.fWriteAlignment),
        fAggregatorSendsKey(config.fSendKey),
        fReduceRootContention(config.fReduceRootContention),
        fUseGlobalOffset(config.fUseGlobalOffset) {
    fMetrics = RNTupleMetrics("RPageSinkMPI");
    fCounters = std::make_unique<RCounters>(RCounters{
        *fMetrics.MakeCounter<RNTupleAtomicCounter *>(
            "nPageCommitted", "", "number of pages committed to storage"),
        *fMetrics.MakeCounter<RNTupleAtomicCounter *>(
            "szWritePayload", "B", "volume written for committed pages"),
        *fMetrics.MakeCounter<RNTupleAtomicCounter *>(
            "timeWallWrite", "ns", "wall clock time spent writing"),
        *fMetrics.MakeCounter<RNTupleAtomicCounter *>(
            "timeWallCommAggregator", "ns",
            "wall clock time spent communicating with the aggregator"),
        *fMetrics.MakeCounter<RNTupleAtomicCounter *>(
            "timeWallGlobalOffset", "ns",
            "wall clock time spent maintaining the global offset"),
        *fMetrics.MakeCounter<RNTupleTickCounter<RNTupleAtomicCounter> *>(
            "timeCpuWrite", "ns", "CPU time spent writing"),
        *fMetrics.MakeCounter<RNTupleTickCounter<RNTupleAtomicCounter> *>(
            "timeCpuCommAggregator", "ns",
            "CPU time spent communicating with the aggregator"),
        *fMetrics.MakeCounter<RNTupleTickCounter<RNTupleAtomicCounter> *>(
            "timeCpuGlobalOffset", "ns",
            "CPU time spent maintaining the global offset")});

    MPI_Comm_dup(comm, &fComm);

    if (!fSendData) {
      std::align_val_t blockAlign{fWriteAlignment};
      fBlock = static_cast<unsigned char *>(
          ::operator new[](kProcessWriteBufferSize, blockAlign));
      memset(fBlock, 0, kProcessWriteBufferSize);
    }

    MPI_Comm_rank(fComm, &fRank);
    MPI_Comm_size(fComm, &fSize);

    if (fRank == root) {
      fAggregator =
          std::make_unique<RNTupleWriterMPIAggregator>(config, root, fComm);
      fMetrics.ObserveMetrics(fAggregator->GetMetrics());
      if (!fUseGlobalOffset) {
        fAggregatorThread =
            std::thread([this]() { fAggregator->Collect(fSize); });
      }
    }

    if (fUseGlobalOffset) {
      if (fUseGlobalOffset == RNTupleWriterMPI::kOneSidedCommunication) {
        MPI_Aint winSize = 0;
        if (fRank == fRoot) {
          winSize = sizeof(std::uint64_t);
        }

        // Configure the window that we need.
        MPI_Info winInfo;
        MPI_Info_create(&winInfo);
        // We want to use passive target communication between MPI_Win_lock and
        // MPI_Win_unlock.
        MPI_Info_set(winInfo, "no_locks", "false");
        // It's not clear if the following settings have an effect since we use
        // MPI_Fetch_and_op and not MPI_Get_accumulate. In any case, we don't
        // need ordering guarantees since the counter is a single std::uint64_t.
        MPI_Info_set(winInfo, "accumulate_ordering", "none");
        // We don't use MPI_NO_OP.
        MPI_Info_set(winInfo, "accumulate_ops", "same_op");
        // The counter is a single std::uint64_t.
        MPI_Info_set(winInfo, "mpi_accumulate_granularity", "8");
        // The argument disp_unit = 1 is identical on all MPI processes.
        MPI_Info_set(winInfo, "same_disp_unit", "true");

        // Set up the window.
        MPI_Win_allocate(winSize, /*disp_unit=*/1, winInfo, fComm, &fOffsetPtr,
                         &fOffsetWindow);
        // Free the info object, not needed anymore.
        MPI_Info_free(&winInfo);

        if (fRank == fRoot) {
          MPI_Win_lock(MPI_LOCK_EXCLUSIVE, fRoot, /*assert=*/0, fOffsetWindow);
          *fOffsetPtr = fAggregator->GetCurrentOffset();
          MPI_Win_unlock(fRoot, fOffsetWindow);
        }
      } else {
        assert(fUseGlobalOffset == RNTupleWriterMPI::kFileLocks ||
               fUseGlobalOffset == RNTupleWriterMPI::kFileLocksSame);
        if (fRank == fRoot) {
          assert(fAggregator);

          // At this point, the aggregator did not write the header yet and it
          // won't until we call CommitDataset(). So just put the global offset
          // at a fixed position.
          OpenFile();
          OpenGlobalOffsetFile(true);
          const GlobalOffsetType offset = fAggregator->GetCurrentOffset();
          ssize_t written = pwrite(fGlobalOffsetFileDes, &offset,
                                   sizeof(offset), kGlobalOffsetOff);
          if (written != sizeof(offset)) {
            throw ROOT::RException(R__FAIL("pwrite() failed"));
          }
        }
      }
      MPI_Barrier(fComm);

      // After the barrier, we are guaranteed that the root created the file and
      // we can open it for parallel writing.
      OpenFile();
      if (fUseGlobalOffset == RNTupleWriterMPI::kFileLocks ||
          fUseGlobalOffset == RNTupleWriterMPI::kFileLocksSame) {
        OpenGlobalOffsetFile();
      }
    }
  }

  ~RPageSinkMPI() final {
    assert(fFileDes < 0 && "CommitDataset() should be called");
    assert(fGlobalOffsetFileDes < 0 && "CommitDataset() should be called");

    if (fBlock) {
      std::align_val_t blockAlign{fWriteAlignment};
      ::operator delete[](fBlock, blockAlign);
    }

    MPI_Comm_free(&fComm);
  }

  void OpenFile() {
    if (fFileDes >= 0) {
      return;
    }

    int flags = 0;
    if (fUseGlobalOffset == RNTupleWriterMPI::kFileLocksSame) {
      // If we use a single file, we need to read and write.
      flags = O_RDWR;
    } else {
      flags = O_WRONLY;
    }
#ifdef O_LARGEFILE
    // Add the equivalent flag that is passed by fopen64.
    flags |= O_LARGEFILE;
#endif
    if (fOptions->GetUseDirectIO()) {
      flags |= O_DIRECT;
    }
    fFileDes = open(fStorage.c_str(), flags, 0666);
    if (fFileDes < 0) {
      throw ROOT::RException(R__FAIL("open() failed"));
    }
  }

  std::string GetOffsetStorage() const { return fStorage + ".offset"; }

  void OpenGlobalOffsetFile(bool create = false) {
    if (fGlobalOffsetFileDes >= 0) {
      return;
    }
    if (fUseGlobalOffset == RNTupleWriterMPI::kFileLocksSame) {
      assert(fFileDes >= 0);
      fGlobalOffsetFileDes = fFileDes;
      return;
    }
    assert(fUseGlobalOffset == RNTupleWriterMPI::kFileLocks);

    std::string offsetStorage = GetOffsetStorage();
    int flags = O_RDWR;
    if (create) {
      flags |= O_CREAT | O_EXCL;
    }
    fGlobalOffsetFileDes = open(offsetStorage.c_str(), flags, 0666);
    if (fGlobalOffsetFileDes < 0) {
      throw ROOT::RException(R__FAIL("open() failed (for global offset file)"));
    }
  }

  std::uint64_t GetAndIncrementOffset(std::uint64_t size) {
    RNTupleAtomicTimer timer(fCounters->fTimeWallGlobalOffset,
                             fCounters->fTimeCpuGlobalOffset);

    assert(fUseGlobalOffset);
    GlobalOffsetType offset;
    if (fUseGlobalOffset == RNTupleWriterMPI::kOneSidedCommunication) {
      assert(fOffsetWindow != MPI_WIN_NULL);

      MPI_Win_lock(MPI_LOCK_EXCLUSIVE, fRoot, /*assert=*/0, fOffsetWindow);
      MPI_Fetch_and_op(&size, &offset, kGlobalOffsetDatatype, fRoot, 0, MPI_SUM,
                       fOffsetWindow);
      MPI_Win_unlock(fRoot, fOffsetWindow);
    } else {
      assert(fUseGlobalOffset == RNTupleWriterMPI::kFileLocks ||
             fUseGlobalOffset == RNTupleWriterMPI::kFileLocksSame);
      assert(fGlobalOffsetFileDes >= 0);

      // Increment the counter: lock the whole file
      struct flock fl;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      fl.l_start = kGlobalOffsetOff;
      fl.l_len = sizeof(offset);
      fl.l_pid = getpid();
      if (fcntl(fGlobalOffsetFileDes, F_SETLKW, &fl) == -1) {
        throw ROOT::RException(
            R__FAIL(std::string("lock failed: ") + strerror(errno)));
      }

      ssize_t read = pread(fGlobalOffsetFileDes, &offset, sizeof(offset),
                           kGlobalOffsetOff);
      if (read != sizeof(offset)) {
        throw ROOT::RException(
            R__FAIL(std::string("read failed: ") + strerror(errno)));
      }
      const GlobalOffsetType update = offset + size;
      ssize_t written = pwrite(fGlobalOffsetFileDes, &update, sizeof(update),
                               kGlobalOffsetOff);
      if (written != sizeof(update)) {
        throw ROOT::RException(
            R__FAIL(std::string("write failed: ") + strerror(errno)));
      }

      // Release the lock.
      fl.l_type = F_UNLCK;
      if (fcntl(fGlobalOffsetFileDes, F_SETLKW, &fl) == -1) {
        throw ROOT::RException(
            R__FAIL(std::string("unlock failed: ") + strerror(errno)));
      }
    }

    return offset;
  }

  const RNTupleDescriptor &GetDescriptor() const final {
    static RNTupleDescriptor descriptor;
    return descriptor;
  }

  NTupleSize_t GetNEntries() const final { return 0; }

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
    throw ROOT::RException(
        R__FAIL("UpdateSchema not supported via RPageSinkMPI"));
  }
  void UpdateExtraTypeInfo(const RExtraTypeInfoDescriptor &) final {
    throw ROOT::RException(
        R__FAIL("UpdateExtraTypeInfo not supported via RPageSinkMPI"));
  }

  void CommitSuppressedColumn(ColumnHandle_t columnHandle) final {
    fBufferedColumns.at(columnHandle.fPhysicalId).fIsSuppressed = true;
  }
  void CountSealedPage(const RPageStorage::RSealedPage &sealedPage,
                       DescriptorId_t columnId) {
    fNPages++;
    fSumSealedPages += sealedPage.GetBufferSize();

    const auto bitsOnStorage = fDescriptorBuilder.GetDescriptor()
                                   .GetColumnDescriptor(columnId)
                                   .GetBitsOnStorage();
    const auto bytesPacked =
        (bitsOnStorage * sealedPage.GetNElements() + 7) / 8;
    fSumBytesPacked += bytesPacked;
  }
  void CommitPage(ColumnHandle_t columnHandle, const RPage &page) final {
    assert(!fOptions->GetUseBufferedWrite());
    const auto columnId = columnHandle.fPhysicalId;
    auto &pageBuf = fBufferedColumns.at(columnId).fPages.emplace_back();
    auto bufferSize =
        page.GetNBytes() +
        GetWriteOptions().GetEnablePageChecksums() * kNBytesPageChecksum;
    pageBuf.fBuffer =
        std::unique_ptr<unsigned char[]>(new unsigned char[bufferSize]);

    RSealPageConfig config;
    config.fPage = &page;
    config.fElement = columnHandle.fColumn->GetElement();
    config.fCompressionSettings = GetWriteOptions().GetCompression();
    config.fWriteChecksum = GetWriteOptions().GetEnablePageChecksums();
    config.fAllowAlias = false;
    config.fBuffer = pageBuf.fBuffer.get();
    pageBuf.fSealedPage = SealPage(config);
    CountSealedPage(pageBuf.fSealedPage, columnId);
  }
  void CommitSealedPage(DescriptorId_t, const RSealedPage &) final {
    throw ROOT::RException(
        R__FAIL("should never commit a single sealed page via RPageSinkMPI"));
  }
  void
  CommitSealedPageV(std::span<RPageStorage::RSealedPageGroup> ranges) final {
    assert(fOptions->GetUseBufferedWrite());
    for (auto &range : ranges) {
      if (range.fFirst == range.fLast) {
        // Skip empty ranges, they might not have a physical column ID!
        continue;
      }

      const auto columnId = range.fPhysicalColumnId;
      auto &columnBuf = fBufferedColumns.at(columnId);
      for (auto sealedPageIt = range.fFirst; sealedPageIt != range.fLast;
           ++sealedPageIt) {
        const auto nElements = sealedPageIt->GetNElements();
        columnBuf.fNElements += nElements;

        auto &pageBuf = columnBuf.fPages.emplace_back();
        // We can just copy the sealed page: The outer RPageSinkBuf will keep
        // the buffers around after CommitCluster.
        pageBuf.fSealedPage = *sealedPageIt;
        CountSealedPage(pageBuf.fSealedPage, columnId);
      }
    }
  }

  RStagedCluster StageCluster(NTupleSize_t) final {
    throw ROOT::RException(
        R__FAIL("staged cluster committing not supported via RPageSinkMPI"));
  }
  void CommitStagedClusters(std::span<RStagedCluster>) final {
    throw ROOT::RException(
        R__FAIL("staged cluster committing not supported via RPageSinkMPI"));
  }

  /// Build a RClusterDescriptor based on the buffered columns and pages and
  /// return its clusterId.
  DescriptorId_t BuildCluster(NTupleSize_t nNewEntries, std::uint64_t offset) {
    const auto &descriptor = fDescriptorBuilder.GetDescriptor();
    DescriptorId_t clusterId = descriptor.GetNActiveClusters();

    RClusterDescriptorBuilder clusterBuilder;
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
          pageInfo.SetNElements(pageBuf.fSealedPage.GetNElements());
          pageInfo.GetLocator().SetPosition(offset);
          pageInfo.GetLocator().SetNBytesOnStorage(
              pageBuf.fSealedPage.GetDataSize());
          pageInfo.SetHasChecksum(pageBuf.fSealedPage.GetHasChecksum());
          pageRange.GetPageInfos().emplace_back(pageInfo);
          offset += pageBuf.fSealedPage.GetBufferSize();
        }
        pageRange.SetPhysicalColumnId(i);
        // First element index is left unset.
        int firstElementIndex = 0;
        int compressionSettings = GetWriteOptions().GetCompression();
        clusterBuilder.CommitColumnRange(i, firstElementIndex,
                                         compressionSettings, pageRange);
      }
    }
    fDescriptorBuilder.AddCluster(clusterBuilder.MoveDescriptor().Unwrap());

    return clusterId;
  }

  /// Serialize the RClusterDescriptor and potentially append the sealed pages.
  std::vector<unsigned char> PrepareSendBuffer(DescriptorId_t clusterId) {
    const auto &descriptor = fDescriptorBuilder.GetDescriptor();
    DescriptorId_t physClusterIDs[] = {
        fSerializationContext.MapClusterId(clusterId)};
    auto szPageList =
        RNTupleSerializer::SerializePageList(
            nullptr, descriptor, physClusterIDs, fSerializationContext)
            .Unwrap();

    auto szBuffer = szPageList;
    if (fSendData) {
      szBuffer += fSumSealedPages;
    }
    std::vector<unsigned char> buffer(szBuffer);
    RNTupleSerializer::SerializePageList(buffer.data(), descriptor,
                                         physClusterIDs, fSerializationContext);

    if (fSendData) {
      // Append all sealed page buffers.
      unsigned char *ptr = buffer.data() + szPageList;
      for (auto &columnBuf : fBufferedColumns) {
        for (auto &pageBuf : columnBuf.fPages) {
          auto sealedBufferSize = pageBuf.fSealedPage.GetBufferSize();
          memcpy(ptr, pageBuf.fSealedPage.GetBuffer(), sealedBufferSize);
          ptr += sealedBufferSize;
        }
      }
      assert(ptr == buffer.data() + szBuffer);
    }

    return buffer;
  }

  /// Write all sealed page in order. fBlock may already have data before
  /// offset % fWriteAlignment, for example the key header.
  void WriteSealedPages(std::uint64_t offset) {
    assert(fFileDes >= 0 && "OpenFile() should be called");

    std::uint64_t blockOffset = offset - offset % fWriteAlignment;

    RNTupleAtomicTimer timer(fCounters->fTimeWallWrite,
                             fCounters->fTimeCpuWrite);

    for (auto &columnBuf : fBufferedColumns) {
      for (auto &pageBuf : columnBuf.fPages) {
        auto nBytesPage = pageBuf.fSealedPage.GetBufferSize();
        const unsigned char *buffer =
            static_cast<const unsigned char *>(pageBuf.fSealedPage.GetBuffer());
        while (nBytesPage > 0) {
          std::uint64_t posInBlock = offset - blockOffset;
          if (posInBlock >= kProcessWriteBufferSize) {
            // Write the block.
            std::size_t retval =
                pwrite(fFileDes, fBlock, kProcessWriteBufferSize, blockOffset);
            if (retval != kProcessWriteBufferSize)
              throw ROOT::RException(
                  R__FAIL(std::string("write failed: ") + strerror(errno)));

            // Null the buffer contents for good measure.
            memset(fBlock, 0, kProcessWriteBufferSize);
            posInBlock = offset % fWriteAlignment;
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
      // Round up to a multiple of fWriteAlignment.
      lastBlockSize += fWriteAlignment - 1;
      lastBlockSize = (lastBlockSize / fWriteAlignment) * fWriteAlignment;
      std::size_t retval = pwrite(fFileDes, fBlock, lastBlockSize, blockOffset);
      if (retval != lastBlockSize)
        throw ROOT::RException(
            R__FAIL(std::string("write failed: ") + strerror(errno)));

      // Null the buffer contents for good measure.
      memset(fBlock, 0, kProcessWriteBufferSize);
    }
  }

  std::uint64_t CommitCluster(NTupleSize_t nNewEntries) final {
    const auto &descriptor = fDescriptorBuilder.GetDescriptor();

    std::uint64_t offset = 0;
    if (fUseGlobalOffset) {
      auto totalSize = kBlobKeyLen + fSumSealedPages;
      std::uint64_t padding = 0;
      if (totalSize % fWriteAlignment != 0) {
        padding += fWriteAlignment - totalSize % fWriteAlignment;
      }
      offset = GetAndIncrementOffset(totalSize + padding);
      R__ASSERT(offset % fWriteAlignment == 0);

      // Write the key by prepending it into the fBlock buffer.
      RNTupleFileWriter::PrepareBlobKey(offset, fSumSealedPages + padding,
                                        fSumBytesPacked, fBlock);
      offset += kBlobKeyLen;
    }

    DescriptorId_t clusterId = BuildCluster(nNewEntries, offset);

    if (!fUseGlobalOffset) {
      // Serialize the cluster and send with MPI.
      auto buffer = PrepareSendBuffer(clusterId);
      unsigned char recvBuf[sizeof(std::uint64_t) + kBlobKeyLen];
      MPI_Status status;
      {
        RNTupleAtomicTimer timer(fCounters->fTimeWallCommAggregator,
                                 fCounters->fTimeCpuCommAggregator);

        MPI_Request req[2];

        MPI_Isend(buffer.data(), buffer.size(), MPI_BYTE, fRoot, kTagAggregator,
                  fComm, &req[0]);

        // Get back the reply, the message is empty unless we did not send the
        // data.
        MPI_Irecv(&recvBuf[0], sizeof(recvBuf), MPI_BYTE, fRoot, kTagOffset,
                  fComm, &req[1]);

        if (fReduceRootContention && fAggregator) {
          fAggregator->WaitForSignal();
        }

        MPI_Wait(&req[0], MPI_STATUS_IGNORE);
        MPI_Wait(&req[1], &status);
      }

      if (!fSendData) {
        // After the aggregator replied, we are guaranteed that it created the
        // file and we can open it for parallel writing.
        OpenFile();

#ifndef NDEBUG
        int count;
        MPI_Get_count(&status, MPI_BYTE, &count);
        if (!fAggregatorSendsKey) {
          assert(count == sizeof(std::uint64_t));
        } else {
          assert(count == sizeof(std::uint64_t) + kBlobKeyLen);
        }
#endif
        RNTupleSerializer::DeserializeUInt64(&recvBuf[0], offset);

        if (!fAggregatorSendsKey) {
          assert(offset % fWriteAlignment == 0);
        } else {
          assert(offset % fWriteAlignment == kBlobKeyLen);
          // If the aggregator sent the key, write it now.
          static_assert(kBlobKeyLen < kProcessWriteBufferSize);
          memcpy(fBlock, &recvBuf[sizeof(std::uint64_t)], kBlobKeyLen);
        }
      }
    }

    if (!fSendData) {
      WriteSealedPages(offset);
    }

    // Clean up all buffered columns and pages.
    for (auto &columnBuf : fBufferedColumns) {
      columnBuf.fPages.clear();
      columnBuf.fNElements = 0;
      columnBuf.fIsSuppressed = false;
    }

    fCounters->fNPageCommitted.Add(fNPages);
    fCounters->fSzWritePayload.Add(fSumSealedPages);

    const auto sumSealedPages = fSumSealedPages;
    fNPages = 0;
    fSumSealedPages = 0;
    fSumBytesPacked = 0;

    return sumSealedPages;
  }
  void CommitClusterGroup() final {
    // TODO or ignored?
  }

  struct GatheredClusters {
    std::unique_ptr<int[]> nClusters;
    std::unique_ptr<int[]> pageListsSizes;
    std::unique_ptr<int[]> pageListsDispls;
    std::unique_ptr<unsigned char[]> pageListsBuffer;
  };

  void AppendClusters(const GatheredClusters &clusters) {
    assert(fAggregator);

    // Deserialize all clusters: Create temporary descriptors to deserialize
    // the page lists and for each cluster remember the offset of one page
    // to establish a linear order.
    std::size_t totalNClusters = 0;
    std::vector<RNTupleDescriptor> descriptors(fSize);
    std::vector<std::vector<std::uint64_t>> clusterOffsets(fSize);
    for (int i = 0; i < fSize; i++) {
      const auto nClusters = clusters.nClusters[i];
      totalNClusters += nClusters;

      RNTupleDescriptorBuilder descriptorBuilder;
      descriptorBuilder.SetNTuple("ntuple", "");
      RClusterGroupDescriptorBuilder cgBuilder;
      cgBuilder.ClusterGroupId(0).NClusters(nClusters);
      descriptorBuilder.AddClusterGroup(cgBuilder.MoveDescriptor().Unwrap());
      descriptors[i] = descriptorBuilder.MoveDescriptor();
      const auto displ = clusters.pageListsDispls[i];
      RNTupleSerializer::DeserializePageList(
          &clusters.pageListsBuffer[displ], clusters.pageListsSizes[i], 0,
          descriptors[i], RNTupleSerializer::EDescriptorDeserializeMode::kRaw);

      auto &offsets = clusterOffsets[i];
      offsets.reserve(nClusters);
      for (DescriptorId_t clusterId = 0; clusterId < nClusters; clusterId++) {
        const auto &clusterDescriptor =
            descriptors[i].GetClusterDescriptor(clusterId);
        for (const auto &columnRange :
             clusterDescriptor.GetColumnRangeIterable()) {
          if (columnRange.IsSuppressed()) {
            continue;
          }

          const auto &pageRange =
              clusterDescriptor.GetPageRange(columnRange.GetPhysicalColumnId());
          if (pageRange.GetPageInfos().empty()) {
            continue;
          }

          const auto &firstPage = pageRange.GetPageInfos().front();
          offsets.push_back(
              firstPage.GetLocator().GetPosition<std::uint64_t>());
          break;
        }
      }
    }

    // Zip clusters into aggregator, ordered by their offset in the file.
    std::vector<DescriptorId_t> nextCluster(fSize);
    std::size_t clustersHandled = 0;
    while (clustersHandled < totalNClusters) {
      std::uint64_t minOffset = 0;
      int minOffsetIdx = -1;
      for (int i = 0; i < fSize; i++) {
        if (nextCluster[i] >= clusters.nClusters[i]) {
          continue;
        }

        if (minOffsetIdx == -1 ||
            clusterOffsets[i][nextCluster[i]] < minOffset) {
          minOffset = clusterOffsets[i][nextCluster[i]];
          minOffsetIdx = i;
        }
      }
      assert(minOffsetIdx != -1);

      fAggregator->AppendCluster(descriptors[minOffsetIdx].GetClusterDescriptor(
          nextCluster[minOffsetIdx]));
      nextCluster[minOffsetIdx]++;
      clustersHandled++;
    }
  }

  /// Gather clusters from ranks on the root.
  void GatherClusters() {
    // On all ranks, serialize the RClusterDescriptor.
    const auto &descriptor = fDescriptorBuilder.GetDescriptor();
    const auto nClusters = descriptor.GetNActiveClusters();
    std::vector<DescriptorId_t> physClusterIDs;
    physClusterIDs.reserve(nClusters);
    for (DescriptorId_t i = 0; i < nClusters; ++i) {
      physClusterIDs.emplace_back(fSerializationContext.MapClusterId(i));
    }
    auto szPageList =
        RNTupleSerializer::SerializePageList(
            nullptr, descriptor, physClusterIDs, fSerializationContext)
            .Unwrap();

    std::unique_ptr<unsigned char[]> pageListBuffer(
        new unsigned char[szPageList]);
    RNTupleSerializer::SerializePageList(pageListBuffer.get(), descriptor,
                                         physClusterIDs, fSerializationContext);

    // Send the number of clusters and the sizes of the page list to the root.
    int nClustersInt = static_cast<int>(nClusters);
    int szPageListInt = static_cast<int>(szPageList);
    GatheredClusters clusters;
    if (fRank == fRoot) {
      clusters.nClusters.reset(new int[fSize]);
      clusters.pageListsSizes.reset(new int[fSize]);
    }
    MPI_Gather(&nClustersInt, 1, MPI_INT, clusters.nClusters.get(), 1, MPI_INT,
               fRoot, fComm);
    MPI_Gather(&szPageListInt, 1, MPI_INT, clusters.pageListsSizes.get(), 1,
               MPI_INT, fRoot, fComm);

    // The root allocates appropriate buffers and receives the page lists.
    if (fRank == fRoot) {
      clusters.pageListsDispls.reset(new int[fSize]);
      clusters.pageListsDispls[0] = 0;
      for (int i = 1; i < fSize; i++) {
        clusters.pageListsDispls[i] =
            clusters.pageListsDispls[i - 1] + clusters.pageListsSizes[i - 1];
      }
      auto sizePageListsBuffer = clusters.pageListsDispls[fSize - 1] +
                                 clusters.pageListsSizes[fSize - 1];
      clusters.pageListsBuffer.reset(new unsigned char[sizePageListsBuffer]);
    }
    MPI_Gatherv(pageListBuffer.get(), szPageListInt, MPI_BYTE,
                clusters.pageListsBuffer.get(), clusters.pageListsSizes.get(),
                clusters.pageListsDispls.get(), MPI_BYTE, fRoot, fComm);

    if (fRank == fRoot) {
      AppendClusters(clusters);
    }
  }

  void CommitDatasetImpl() final {
    if (!fUseGlobalOffset) {
      MPI_Send(NULL, 0, MPI_BYTE, fRoot, kTagAggregator, fComm);
    } else {
      // Gather all serialized clusters. This also synchronizes all ranks, so no
      // additional barrier is needed.
      GatherClusters();

      // At the end, get the final offset and seek the aggregator before writing
      // the metadata.
      if (fAggregator) {
        std::uint64_t offset;
        if (fUseGlobalOffset == RNTupleWriterMPI::kOneSidedCommunication) {
          assert(fOffsetWindow != MPI_WIN_NULL);
          assert(fOffsetPtr != nullptr);
          MPI_Win_lock(MPI_LOCK_EXCLUSIVE, fRoot, /*assert=*/0, fOffsetWindow);
          offset = *fOffsetPtr;
          MPI_Win_unlock(fRoot, fOffsetWindow);
        } else {
          offset = GetAndIncrementOffset(0);
        }
        fAggregator->Seek(offset);
      }
      if (fOffsetWindow != MPI_WIN_NULL) {
        MPI_Win_free(&fOffsetWindow);
      }
      if (fGlobalOffsetFileDes != -1) {
        // fGlobalOffsetFileDes may alias fFileDes, only close it once below.
        // Also only then the root may remove the file.
        if (fGlobalOffsetFileDes != fFileDes) {
          close(fGlobalOffsetFileDes);
          if (fRank == fRoot) {
            // Note: some ranks may still have the file open. This is fine.
            remove(GetOffsetStorage().c_str());
          }
        }
        fGlobalOffsetFileDes = -1;
      }
    }

    // Close the file used for parallel writing.
    if (fFileDes > 0) {
      close(fFileDes);
      fFileDes = -1;
    }
    // On the root, wait for the aggregator and write the metadata.
    if (fAggregatorThread.joinable()) {
      fAggregatorThread.join();
    }
    if (fAggregator) {
      fAggregator->CommitDataset();
      // fAggregator must not be destructed because its metrics can still be
      // queried by the user.
    }
  }
};

} // namespace

std::unique_ptr<ROOT::Experimental::RNTupleWriter>
RNTupleWriterMPI::Recreate(Config config, int root, MPI_Comm comm) {
  int flag = 0;
  MPI_Initialized(&flag);
  if (!flag) {
    throw ROOT::RException(R__FAIL("MPI library was not initialized"));
  }

  if (config.fWriteAlignment % 4096 != 0) {
    throw ROOT::RException(R__FAIL("write alignment must be multiple of 4096"));
  }

  if (!config.fUseGlobalOffset) {
    if (config.fSendKey && config.fSendData) {
      throw ROOT::RException(
          R__FAIL("sending key requires writing by all processes"));
    }

    int provided = -1;
    MPI_Query_thread(&provided);
    if (provided != MPI_THREAD_MULTIPLE) {
      throw ROOT::RException(R__FAIL(
          "RNTupleWriterMPI with aggregator requires MPI_THREAD_MULTIPLE"));
    }
  } else {
    if (config.fSendData || config.fSendKey) {
      throw ROOT::RException(
          R__FAIL("cannot send data or key with global offset"));
    }
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
