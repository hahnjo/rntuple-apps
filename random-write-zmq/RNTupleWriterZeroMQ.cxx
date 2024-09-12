// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterZeroMQ.hxx"

#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleSerialize.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageAllocator.hxx>
#include <ROOT/RPageSinkBuf.hxx>
#include <ROOT/RPageStorage.hxx>

#include <zmq.h>

#include <string>
#include <string_view>
#include <utility>

namespace {

using ROOT::Experimental::RException;

/// Receive a message from the ZeroMQ socket. This method initializes msg and
/// takes care of retries when interrupted.
void ZMQMsgRecv(zmq_msg_t *msg, void *socket) {
  int rc = zmq_msg_init(msg);
  if (rc) {
    throw RException(R__FAIL("zmq_msg_init() failed"));
  }
  int nbytes;
  while (true) {
    nbytes = zmq_msg_recv(msg, socket, 0);
    if (nbytes == -1) {
      if (errno == EINTR) {
        // Try again.
        continue;
      }
      throw RException(R__FAIL("zmq_msg_recv() failed"));
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
      throw RException(R__FAIL("zmq_send() failed"));
    }
    break;
  }
}

using ROOT::Experimental::ClusterSize_t;
using ROOT::Experimental::DescriptorId_t;
using ROOT::Experimental::NTupleSize_t;
using ROOT::Experimental::RClusterDescriptor;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::Internal::RClusterDescriptorBuilder;
using ROOT::Experimental::Internal::RClusterGroupDescriptorBuilder;
using ROOT::Experimental::Internal::RColumn;
using ROOT::Experimental::Internal::RColumnDescriptorBuilder;
using ROOT::Experimental::Internal::RFieldDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleDescriptorBuilder;
using ROOT::Experimental::Internal::RNTupleModelChangeset;
using ROOT::Experimental::Internal::RNTupleSerializer;
using ROOT::Experimental::Internal::RPage;
using ROOT::Experimental::Internal::RPagePersistentSink;
using ROOT::Experimental::Internal::RPageSink;
using ROOT::Experimental::Internal::RPageStorage;

/// A page sink that sends all data to the RNTupleWriterZeroMQ server.
class RPageSinkZeroMQ : public RPageSink {
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
  /// server; see CommitCluster.
  RNTupleSerializer::RContext fSerializationContext;
  /// A dummy descriptor builder to send information about a cluster to the
  /// server; see CommitCluster.
  RNTupleDescriptorBuilder fDescriptorBuilder;

  /// The ZeroMQ context for this client.
  void *fContext;
  /// The ZeroMQ socket connected to the server.
  void *fSocket;

public:
  RPageSinkZeroMQ(const RNTupleWriterZeroMQ::Config &config)
      : RPageSink(config.fNTupleName, config.fOptions) {
    fContext = zmq_ctx_new();
    if (!fContext) {
      throw RException(R__FAIL("zmq_ctx_new() failed"));
    }
    fSocket = zmq_socket(fContext, ZMQ_REQ);
    if (!fSocket) {
      throw RException(R__FAIL("zmq_socket() failed"));
    }
    std::string endpoint(config.fEndpoint);
    int rc = zmq_connect(fSocket, endpoint.c_str());
    if (rc) {
      throw RException(R__FAIL("zmq_connect() failed"));
    }
  }

  ~RPageSinkZeroMQ() final {
    zmq_close(fSocket);
    zmq_ctx_destroy(fContext);
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
    auto &fieldZero = model.GetFieldZero();
    fDescriptorBuilder.AddField(RFieldDescriptorBuilder::FromField(fieldZero)
                                    .FieldId(0)
                                    .MakeDescriptor()
                                    .Unwrap());
    fieldZero.SetOnDiskId(0);
    model.GetProjectedFields().GetFieldZero()->SetOnDiskId(0);

    const auto &descriptor = fDescriptorBuilder.GetDescriptor();
    for (auto &f : model.GetFieldZero()) {
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
    throw RException(R__FAIL("UpdateSchema not supported via RPageSinkZeroMQ"));
  }
  void UpdateExtraTypeInfo(const RExtraTypeInfoDescriptor &) final {
    throw RException(
        R__FAIL("UpdateExtraTypeInfo not supported via RPageSinkZeroMQ"));
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
    throw RException(R__FAIL(
        "should never commit a single sealed page via RPageSinkZeroMQ"));
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
    throw RException(R__FAIL(
        "staged cluster committing not supported via RPageSinkZeroMQClient"));
  }
  void CommitStagedClusters(std::span<RStagedCluster>) final {
    throw RException(R__FAIL(
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

    auto szBuffer = szPageList + sumSealedPages;
    auto buffer = std::make_unique<unsigned char[]>(szBuffer);
    RNTupleSerializer::SerializePageList(buffer.get(), descriptor,
                                         physClusterIDs, fSerializationContext);

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

    ZMQSend(fSocket, buffer.get(), szBuffer);

    // Get back the reply.
    zmq_msg_t msg;
    ZMQMsgRecv(&msg, fSocket);

    int rc = zmq_msg_close(&msg);
    if (rc) {
      throw RException(R__FAIL("zmq_msg_close() failed"));
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
      throw RException(R__FAIL("final message was not empty"));
    }

    int rc = zmq_msg_close(&msg);
    if (rc) {
      throw RException(R__FAIL("zmq_msg_close() failed"));
    }
  }
};

} // namespace

RNTupleWriterZeroMQ::RNTupleWriterZeroMQ(Config config)
    : fModel(std::move(config.fModel)), fMetrics("RNTupleWriterZeroMQ") {
  fContext = zmq_ctx_new();
  if (!fContext) {
    throw RException(R__FAIL("zmq_ctx_new() failed"));
  }
  fSocket = zmq_socket(fContext, ZMQ_REP);
  if (!fSocket) {
    throw RException(R__FAIL("zmq_socket() failed"));
  }
  std::string endpoint(config.fEndpoint);
  int rc = zmq_bind(fSocket, endpoint.c_str());
  if (rc) {
    throw RException(R__FAIL("zmq_bind() failed"));
  }

  fSink = RPagePersistentSink::Create(config.fNTupleName, config.fStorage,
                                      config.fOptions);
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
        throw RException(R__FAIL("zmq_msg_close() failed"));
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
    RNTupleSerializer::DeserializePageList(msgBuffer, msgSize, 0, descriptor);
    auto &clusterDescriptor = descriptor.GetClusterDescriptor(0);

    // The first 64 bits of the envelope is the type and the length.
    std::uint64_t typeAndSize;
    RNTupleSerializer::DeserializeUInt64(msgBuffer, typeAndSize);
    std::uint64_t envelopeSize = typeAndSize >> 16;

    // Rebuild the list of sealed pages, pointing into the message buffer.
    auto *ptr = static_cast<unsigned char *>(msgBuffer) + envelopeSize;
    std::deque<RPageStorage::SealedPageSequence_t> sealedPagesV;
    std::vector<RPageStorage::RSealedPageGroup> sealedPageGroups;
    auto nColumns = clusterDescriptor.GetColumnRangeIterable().count();
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
        ptr += bytesOnStorage;
      }

      sealedPagesV.push_back(std::move(sealedPages));
      sealedPageGroups.emplace_back(columnId, sealedPagesV.back().cbegin(),
                                    sealedPagesV.back().cend());

      columnId++;
    }
    assert(ptr == static_cast<unsigned char *>(msgBuffer) + msgSize);

    fSink->CommitSealedPageV(sealedPageGroups);
    fSink->CommitCluster(clusterDescriptor.GetNEntries());

    rc = zmq_msg_close(&msg);
    if (rc) {
      throw RException(R__FAIL("zmq_msg_close() failed"));
    }

    // Send the response.
    std::size_t size = 0;

    ZMQSend(fSocket, NULL, size);
  }
}

std::unique_ptr<RNTupleWriterZeroMQ>
RNTupleWriterZeroMQ::Recreate(Config config) {
  return std::unique_ptr<RNTupleWriterZeroMQ>(
      new RNTupleWriterZeroMQ(std::move(config)));
}

std::unique_ptr<ROOT::Experimental::RNTupleWriter>
RNTupleWriterZeroMQ::CreateWorkerWriter(Config config) {
  std::unique_ptr<ROOT::Experimental::Internal::RPageSink> sink =
      std::make_unique<RPageSinkZeroMQ>(config);
  if (config.fOptions.GetUseBufferedWrite()) {
    sink = std::make_unique<ROOT::Experimental::Internal::RPageSinkBuf>(
        std::move(sink));
  }

  return ROOT::Experimental::Internal::CreateRNTupleWriter(
      std::move(config.fModel), std::move(sink));
}
