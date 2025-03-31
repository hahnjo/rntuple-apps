// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterZeroMQ.hxx"

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

#include <zmq.h>

#include <string>
#include <string_view>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

/// The alignment for write boundaries between server and client. Because it is
/// used as the write buffer size on the server, it must be a multiple of 4096
/// (see RNTupleWriteOptions).
static constexpr std::size_t kServerClientWriteAlignment = 4096;
static constexpr std::size_t kClientWriteBufferSize = 4 * 1024 * 1024;

namespace {

/// Receive a message from the ZeroMQ socket. This method initializes msg and
/// takes care of retries when interrupted.
void ZMQMsgRecv(zmq_msg_t *msg, void *socket) {
  int rc = zmq_msg_init(msg);
  if (rc) {
    throw ROOT::RException(R__FAIL("zmq_msg_init() failed"));
  }
  int nbytes;
  while (true) {
    nbytes = zmq_msg_recv(msg, socket, 0);
    if (nbytes == -1) {
      if (errno == EINTR) {
        // Try again.
        continue;
      }
      throw ROOT::RException(R__FAIL("zmq_msg_recv() failed"));
    }
    break;
  }
}

/// Send a message via the ZeroMQ socket. This method takes care of retries when
/// interrupted.
void ZMQSend(void *socket, const void *buf, std::size_t len) {
  while (true) {
    int nbytes = zmq_send(socket, buf, len, 0);
    if (static_cast<std::size_t>(nbytes) != len) {
      if (nbytes == -1 && errno == EINTR) {
        // Try again.
        continue;
      }
      throw ROOT::RException(R__FAIL("zmq_send() failed"));
    }
    break;
  }
}

using ROOT::DescriptorId_t;
using ROOT::NTupleSize_t;
using ROOT::RNTupleLocator;
using ROOT::Experimental::RClusterDescriptor;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::Detail::RNTupleAtomicTimer;
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
/// RNTupleWriterZeroMQ server.
class RPageSinkZeroMQServer final : public RPagePersistentSink {
  /// The file writer to write the ntuple.
  std::unique_ptr<RNTupleFileWriter> fWriter;
  /// The last offset in the file that was reserved in CommitSealedPageVImpl.
  std::uint64_t fLastOffset = 0;
  /// The current offset in the file.
  std::uint64_t fCurrentOffset = 0;
  /// The key buffer when sending to the processes.
  unsigned char fKeyBuffer[kBlobKeyLen];
  /// Whether the clients are expected to send the payload data.
  bool fClientsSendData;
  /// Whether to send the key to the clients instead of writing.
  bool fSendKey;

public:
  RPageSinkZeroMQServer(const RNTupleWriterZeroMQ::Config &config)
      : RPagePersistentSink(config.fNTupleName, config.fOptions),
        fClientsSendData(config.fSendData), fSendKey(config.fSendKey) {
    EnableDefaultMetrics("RPageSinkZeroMQ");
    // No support for merging pages at the moment
    fFeatures.fCanMergePages = false;

    // Create the file writer, but force the write buffer size to avoid
    // overlapping writes on server and client.
    // TODO: This is pessimistic for writing the header and footer...
    ROOT::RNTupleWriteOptions options = config.fOptions;
    if (!fClientsSendData) {
      options.SetWriteBufferSize(kServerClientWriteAlignment);
    }

    fWriter = RNTupleFileWriter::Recreate(
        config.fNTupleName, config.fStorage,
        RNTupleFileWriter::EContainerFormat::kTFile, options);
  }

  std::uint64_t GetLastOffset() const { return fLastOffset; }
  const unsigned char *GetKeyBuffer() const { return &fKeyBuffer[0]; }

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
    if (!fClientsSendData &&
        fCurrentOffset % kServerClientWriteAlignment != 0) {
      // Insert a dummy blob to make the offset aligned. For this, we need at
      // least kBlobKeyLen bytes to write the key.
      auto dummyOffset = fCurrentOffset + kBlobKeyLen;
      std::size_t bytes = 0;
      if (dummyOffset % kServerClientWriteAlignment != 0) {
        bytes = kServerClientWriteAlignment -
                dummyOffset % kServerClientWriteAlignment;
      }
      offset = fWriter->ReserveBlob(bytes, 0);
      R__ASSERT(offset == dummyOffset);
      fCurrentOffset = offset + bytes;
    }
    R__ASSERT(fClientsSendData ||
              fCurrentOffset % kServerClientWriteAlignment == 0);
  };

  RNTupleLocator CommitPageImpl(ColumnHandle_t, const RPage &) override {
    throw ROOT::RException(
        R__FAIL("should never commit a single page via RPageSinkZeroMQServer"));
    return {};
  }
  RNTupleLocator CommitSealedPageImpl(DescriptorId_t,
                                      const RSealedPage &) override {
    throw ROOT::RException(
        R__FAIL("should never commit a single page via RPageSinkZeroMQServer"));
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
      if (!fClientsSendData) {
        // If the clients write the data directly, we need to pad the reserved
        // buffer accordingly to avoid overlapping writes.
        R__ASSERT(fCurrentOffset % kServerClientWriteAlignment == 0);
        if (!fSendKey) {
          // For the key header, we know that the current offset is aligned and
          // we need to pad until the next alignment boundary.
          padding = kServerClientWriteAlignment - kBlobKeyLen;
          // For the end of the buffer, we again need to pad until the next
          // alignment boundary.
          if (sumSealedPages % kServerClientWriteAlignment != 0) {
            padding += kServerClientWriteAlignment -
                       sumSealedPages % kServerClientWriteAlignment;
          }
        } else {
          // If the clients also write the key, need to factor this in.
          auto totalSize = kBlobKeyLen + sumSealedPages;
          // Then we need to pad until the next alignment boundary.
          if (totalSize % kServerClientWriteAlignment != 0) {
            padding += kServerClientWriteAlignment -
                       totalSize % kServerClientWriteAlignment;
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
      R__ASSERT(fClientsSendData ||
                fCurrentOffset % kServerClientWriteAlignment == 0);

      if (!fClientsSendData) {
        R__ASSERT(offset % kServerClientWriteAlignment == kBlobKeyLen);
        if (!fSendKey) {
          offset += kServerClientWriteAlignment - kBlobKeyLen;
        }
      }
      fLastOffset = offset;

      locators.reserve(nPages);

      for (auto rangeIt = ranges.begin(); rangeIt != ranges.end(); ++rangeIt) {
        for (auto sealedPageIt = rangeIt->fFirst;
             sealedPageIt != rangeIt->fLast; ++sealedPageIt) {
          const auto &sealedPage = *sealedPageIt;
          if (sealedPage.GetBuffer()) {
            assert(fClientsSendData);
            // If the buffer is nullptr, the client did not send the data and
            // will instead write into this offset.
            fWriter->WriteIntoReservedBlob(sealedPage.GetBuffer(),
                                           sealedPage.GetBufferSize(), offset);
          } else {
            assert(!fClientsSendData);
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
    // We don't care about the number of bytes written on the server.
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

} // namespace

void RNTupleWriterZeroMQ::Collect(std::size_t clients) {
  int rc;
  while (clients > 0) {
    zmq_msg_t msg;
    ZMQMsgRecv(&msg, fSocket);
    std::size_t msgSize = zmq_msg_size(&msg);

    if (msgSize == 0) {
      // The client signaled it is done. Send back an empty message.
      clients--;

      rc = zmq_msg_close(&msg);
      if (rc) {
        throw ROOT::RException(R__FAIL("zmq_msg_close() failed"));
      }

      ZMQSend(fSocket, NULL, 0);
      continue;
    }

    // The client wants to commit a cluster.
    void *msgBuffer = zmq_msg_data(&msg);

    // Create a temporary descriptor to deserialize the client's page list.
    RNTupleDescriptorBuilder descriptorBuilder;
    descriptorBuilder.SetNTuple("ntuple", "");
    RClusterGroupDescriptorBuilder cgBuilder;
    cgBuilder.ClusterGroupId(0).NClusters(1);
    descriptorBuilder.AddClusterGroup(cgBuilder.MoveDescriptor().Unwrap());
    auto descriptor = descriptorBuilder.MoveDescriptor();
    RNTupleSerializer::DeserializePageList(
        msgBuffer, msgSize, 0, descriptor,
        RNTupleSerializer::EDescriptorDeserializeMode::kRaw);
    auto &clusterDescriptor = descriptor.GetClusterDescriptor(0);

    unsigned char *ptr = nullptr;
    if (fClientsSendData) {
      // The first 64 bits of the envelope is the type and the length.
      std::uint64_t typeAndSize;
      RNTupleSerializer::DeserializeUInt64(msgBuffer, typeAndSize);
      std::uint64_t envelopeSize = typeAndSize >> 16;

      ptr = static_cast<unsigned char *>(msgBuffer) + envelopeSize;
    }
    std::uint64_t offset = 0;

    // Rebuild the list of sealed pages. If the client sent the data, they will
    // point into the message buffer. Otherwise, the buffer will be the nullptr
    // and RPageSinkZeroMQServer will not write them.
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
    assert(ptr == nullptr ||
           ptr == static_cast<unsigned char *>(msgBuffer) + msgSize);

    fSink->CommitSealedPageV(sealedPageGroups);
    fSink->CommitCluster(clusterDescriptor.GetNEntries());

    rc = zmq_msg_close(&msg);
    if (rc) {
      throw ROOT::RException(R__FAIL("zmq_msg_close() failed"));
    }

    // Send the response.
    std::size_t size = 0;
    unsigned char buffer[sizeof(std::uint64_t) + kBlobKeyLen];
    if (!fClientsSendData) {
      auto *zmqSink = static_cast<RPageSinkZeroMQServer *>(fSink.get());
      std::uint64_t offset = zmqSink->GetLastOffset();
      RNTupleSerializer::SerializeUInt64(offset, &buffer[0]);
      size = sizeof(std::uint64_t);
      if (fSendKey) {
        memcpy(&buffer[size], zmqSink->GetKeyBuffer(), kBlobKeyLen);
        size += kBlobKeyLen;
      }
    }

    ZMQSend(fSocket, &buffer[0], size);
  }
}

namespace {

/// A page sink that sends all data to the RNTupleWriterZeroMQ server.
class RPageSinkZeroMQClient final : public RPageSink {
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

  /// A dummy serialization context to send information about a cluster to the
  /// server; see CommitCluster.
  RNTupleSerializer::RContext fSerializationContext;
  /// A dummy descriptor builder to send information about a cluster to the
  /// server; see CommitCluster.
  RNTupleDescriptorBuilder fDescriptorBuilder;

  /// The ZeroMQ context for this client.
  void *fContext;
  /// The ZeroMQ socket connected to the server.
  void *fSocket;

  /// Storage path for opening the same file as the server.
  std::string fStorage;
  /// The opened file descriptor.
  int fFileDes = -1;
  /// A scratch area to buffer writes to the file.
  unsigned char *fBlock = nullptr;
  /// Whether to send the payload data to the server.
  bool fSendData;
  /// Whether the server sends the key.
  bool fServerSendsKey;

public:
  RPageSinkZeroMQClient(const RNTupleWriterZeroMQ::Config &config)
      : RPageSink(config.fNTupleName, config.fOptions),
        fStorage(config.fStorage), fSendData(config.fSendData),
        fServerSendsKey(config.fSendKey) {
    fContext = zmq_ctx_new();
    if (!fContext) {
      throw ROOT::RException(R__FAIL("zmq_ctx_new() failed"));
    }
    fSocket = zmq_socket(fContext, ZMQ_REQ);
    if (!fSocket) {
      throw ROOT::RException(R__FAIL("zmq_socket() failed"));
    }
    std::string endpoint(config.fEndpoint);
    int rc = zmq_connect(fSocket, endpoint.c_str());
    if (rc) {
      throw ROOT::RException(R__FAIL("zmq_connect() failed"));
    }

    if (!fSendData) {
      std::align_val_t blockAlign{kServerClientWriteAlignment};
      fBlock = static_cast<unsigned char *>(
          ::operator new[](kClientWriteBufferSize, blockAlign));
      memset(fBlock, 0, kClientWriteBufferSize);
    }
  }

  ~RPageSinkZeroMQClient() final {
    assert(fFileDes < 0 && "CommitDataset() should be called");

    if (fBlock) {
      std::align_val_t blockAlign{kServerClientWriteAlignment};
      ::operator delete[](fBlock, blockAlign);
    }

    zmq_close(fSocket);
    zmq_ctx_destroy(fContext);
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
        R__FAIL("UpdateSchema not supported via RPageSinkZeroMQClient"));
  }
  void UpdateExtraTypeInfo(const RExtraTypeInfoDescriptor &) final {
    throw ROOT::RException(
        R__FAIL("UpdateExtraTypeInfo not supported via RPageSinkZeroMQClient"));
  }

  void CommitSuppressedColumn(ColumnHandle_t columnHandle) final {
    fBufferedColumns.at(columnHandle.fPhysicalId).fIsSuppressed = true;
  }
  void CommitPage(ColumnHandle_t columnHandle, const RPage &page) final {
    assert(!fOptions->GetUseBufferedWrite());
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
    config.fCompressionSettings = GetWriteOptions().GetCompression();
    config.fWriteChecksum = GetWriteOptions().GetEnablePageChecksums();
    config.fAllowAlias = false;
    config.fBuffer = pageBuf.fBuffer.get();
    pageBuf.fSealedPage = SealPage(config);
  }
  void CommitSealedPage(DescriptorId_t, const RSealedPage &) final {
    throw ROOT::RException(R__FAIL(
        "should never commit a single sealed page via RPageSinkZeroMQClient"));
  }
  void
  CommitSealedPageV(std::span<RPageStorage::RSealedPageGroup> ranges) final {
    assert(fOptions->GetUseBufferedWrite());
    for (auto &range : ranges) {
      if (range.fFirst == range.fLast) {
        // Skip empty ranges, they might not have a physical column ID!
        continue;
      }

      auto &columnBuf = fBufferedColumns.at(range.fPhysicalColumnId);
      for (auto sealedPageIt = range.fFirst; sealedPageIt != range.fLast;
           ++sealedPageIt) {
        columnBuf.fNElements += sealedPageIt->GetNElements();

        auto &pageBuf = columnBuf.fPages.emplace_back();
        // We can just copy the sealed page: The outer RPageSinkBuf will keep
        // the buffers around after CommitCluster.
        pageBuf.fSealedPage = *sealedPageIt;
      }
    }
  }

  RStagedCluster StageCluster(NTupleSize_t) final {
    throw ROOT::RException(R__FAIL(
        "staged cluster committing not supported via RPageSinkZeroMQClient"));
  }
  void CommitStagedClusters(std::span<RStagedCluster>) final {
    throw ROOT::RException(R__FAIL(
        "staged cluster committing not supported via RPageSinkZeroMQClient"));
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
          pageInfo.SetNElements(pageBuf.fSealedPage.GetNElements());
          // sumSealedPages also serves as the offset into the cluster RBlob.
          pageInfo.GetLocator().SetPosition(sumSealedPages);
          pageInfo.GetLocator().SetNBytesOnStorage(
              pageBuf.fSealedPage.GetDataSize());
          pageInfo.SetHasChecksum(pageBuf.fSealedPage.GetHasChecksum());
          sumSealedPages += pageBuf.fSealedPage.GetBufferSize();
          pageRange.GetPageInfos().emplace_back(pageInfo);
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

    // Serialize the RClusterDescriptor and send via the socket.
    DescriptorId_t physClusterIDs[] = {
        fSerializationContext.MapClusterId(clusterId)};
    auto szPageList =
        RNTupleSerializer::SerializePageList(
            nullptr, descriptor, physClusterIDs, fSerializationContext)
            .Unwrap();

    auto szBuffer = szPageList;
    if (fSendData) {
      szBuffer += sumSealedPages;
    }
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[szBuffer]);
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

    ZMQSend(fSocket, buffer.get(), szBuffer);

    // Get back the reply.
    zmq_msg_t msg;
    ZMQMsgRecv(&msg, fSocket);

    if (!fSendData) {
      // After the server replied, we are guaranteed that it created the file
      // and we can open it for parallel writing.
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
          throw ROOT::RException(R__FAIL("open() failed"));
        }
      }

      if (!fServerSendsKey) {
        assert(zmq_msg_size(&msg) == sizeof(std::uint64_t));
      } else {
        assert(zmq_msg_size(&msg) == sizeof(std::uint64_t) + kBlobKeyLen);
      }
      auto *recv = static_cast<const unsigned char *>(zmq_msg_data(&msg));
      std::uint64_t offset;
      RNTupleSerializer::DeserializeUInt64(recv, offset);
      std::uint64_t blockOffset = offset;

      if (!fServerSendsKey) {
        assert(offset % kServerClientWriteAlignment == 0);
      } else {
        assert(offset % kServerClientWriteAlignment == kBlobKeyLen);
        blockOffset -= kBlobKeyLen;
        // If the server sent the key, write it now.
        static_assert(kBlobKeyLen < kClientWriteBufferSize);
        memcpy(fBlock, &recv[sizeof(std::uint64_t)], kBlobKeyLen);
      }
      assert(blockOffset % kServerClientWriteAlignment == 0);

      // Write the sealed page buffers in the same order.
      for (auto &columnBuf : fBufferedColumns) {
        for (auto &pageBuf : columnBuf.fPages) {
          auto nBytesPage = pageBuf.fSealedPage.GetBufferSize();
          const unsigned char *buffer = static_cast<const unsigned char *>(
              pageBuf.fSealedPage.GetBuffer());
          while (nBytesPage > 0) {
            std::uint64_t posInBlock = offset - blockOffset;
            if (posInBlock >= kClientWriteBufferSize) {
              // Write the block.
              std::size_t retval =
                  pwrite(fFileDes, fBlock, kClientWriteBufferSize, blockOffset);
              if (retval != kClientWriteBufferSize)
                throw ROOT::RException(R__FAIL("pwrite() failed"));

              // Null the buffer contents for good measure.
              memset(fBlock, 0, kClientWriteBufferSize);
              posInBlock = offset % kServerClientWriteAlignment;
              blockOffset = offset - posInBlock;
            }

            std::size_t blockSize = nBytesPage;
            if (blockSize > kClientWriteBufferSize - posInBlock) {
              blockSize = kClientWriteBufferSize - posInBlock;
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
        // Round up to a multiple of kServerClientWriteAlignment.
        lastBlockSize += kServerClientWriteAlignment - 1;
        lastBlockSize = (lastBlockSize / kServerClientWriteAlignment) *
                        kServerClientWriteAlignment;
        std::size_t retval =
            pwrite(fFileDes, fBlock, lastBlockSize, blockOffset);
        if (retval != lastBlockSize)
          throw ROOT::RException(R__FAIL("pwrite() failed"));

        // Null the buffer contents for good measure.
        memset(fBlock, 0, kClientWriteBufferSize);
      }
    }

    int rc = zmq_msg_close(&msg);
    if (rc) {
      throw ROOT::RException(R__FAIL("zmq_msg_close() failed"));
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
    ZMQSend(fSocket, NULL, 0);

    zmq_msg_t msg;
    ZMQMsgRecv(&msg, fSocket);
    std::size_t msgSize = zmq_msg_size(&msg);
    if (msgSize != 0) {
      throw ROOT::RException(R__FAIL("final message was not empty"));
    }

    int rc = zmq_msg_close(&msg);
    if (rc) {
      throw ROOT::RException(R__FAIL("zmq_msg_close() failed"));
    }

    // Close the file used for parallel writing.
    if (fFileDes > 0) {
      close(fFileDes);
      fFileDes = -1;
    }
  }
};

} // namespace

RNTupleWriterZeroMQ::RNTupleWriterZeroMQ(Config config)
    : fModel(std::move(config.fModel)), fMetrics("RNTupleWriterZeroMQ"),
      fClientsSendData(config.fSendData), fSendKey(config.fSendKey) {
  fContext = zmq_ctx_new();
  if (!fContext) {
    throw ROOT::RException(R__FAIL("zmq_ctx_new() failed"));
  }
  fSocket = zmq_socket(fContext, ZMQ_REP);
  if (!fSocket) {
    throw ROOT::RException(R__FAIL("zmq_socket() failed"));
  }
  std::string endpoint(config.fEndpoint);
  int rc = zmq_bind(fSocket, endpoint.c_str());
  if (rc) {
    throw ROOT::RException(R__FAIL("zmq_bind() failed"));
  }

  fSink = std::make_unique<RPageSinkZeroMQServer>(config);
  fModel->Freeze();
  fSink->Init(*fModel);

  fMetrics.ObserveMetrics(fSink->GetMetrics());
}

RNTupleWriterZeroMQ::~RNTupleWriterZeroMQ() {
  // Commit the ntuple to storage.
  fSink->CommitClusterGroup();
  fSink->CommitDataset();

  zmq_close(fSocket);
  zmq_ctx_destroy(fContext);
}

std::unique_ptr<RNTupleWriterZeroMQ>
RNTupleWriterZeroMQ::Recreate(Config config) {
  if (config.fSendKey && config.fSendData) {
    throw ROOT::RException(
        R__FAIL("sending key requires writing by all processes"));
  }

  return std::unique_ptr<RNTupleWriterZeroMQ>(
      new RNTupleWriterZeroMQ(std::move(config)));
}

std::unique_ptr<ROOT::Experimental::RNTupleWriter>
RNTupleWriterZeroMQ::CreateWorkerWriter(Config config) {
  std::unique_ptr<ROOT::Experimental::Internal::RPageSink> sink =
      std::make_unique<RPageSinkZeroMQClient>(config);
  if (config.fOptions.GetUseBufferedWrite()) {
    sink = std::make_unique<ROOT::Experimental::Internal::RPageSinkBuf>(
        std::move(sink));
  }

  return ROOT::Experimental::Internal::CreateRNTupleWriter(
      std::move(config.fModel), std::move(sink));
}
