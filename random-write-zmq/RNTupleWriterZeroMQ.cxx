// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterZeroMQ.hxx"

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

using ROOT::Experimental::DescriptorId_t;
using ROOT::Experimental::NTupleSize_t;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::Internal::RColumn;
using ROOT::Experimental::Internal::RNTupleModelChangeset;
using ROOT::Experimental::Internal::RPage;
using ROOT::Experimental::Internal::RPageSink;

/// A page sink that sends all data to the RNTupleWriterZeroMQ server.
class RPageSinkZeroMQ : public RPageSink {
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

  ColumnHandle_t AddColumn(DescriptorId_t, RColumn &) final { return {}; }
  void InitImpl(RNTupleModel &) final {}
  void UpdateSchema(const RNTupleModelChangeset &, NTupleSize_t) final {
    throw RException(R__FAIL("UpdateSchema not supported via RPageSinkZeroMQ"));
  }
  void UpdateExtraTypeInfo(const RExtraTypeInfoDescriptor &) final {
    throw RException(
        R__FAIL("UpdateExtraTypeInfo not supported via RPageSinkZeroMQ"));
  }

  void CommitSuppressedColumn(ColumnHandle_t) final {
    // TODO
  }
  void CommitPage(ColumnHandle_t, const RPage &) final {
    // TODO
  }
  void CommitSealedPage(DescriptorId_t, const RSealedPage &) final {
    // TODO
  }
  void CommitSealedPageV(std::span<RPageStorage::RSealedPageGroup>) final {
    // TODO
  }

  RStagedCluster StageCluster(NTupleSize_t) final {
    throw RException(R__FAIL(
        "staged cluster committing not supported via RPageSinkZeroMQClient"));
  }
  void CommitStagedClusters(std::span<RStagedCluster>) final {
    throw RException(R__FAIL(
        "staged cluster committing not supported via RPageSinkZeroMQClient"));
  }

  std::uint64_t CommitCluster(NTupleSize_t) final {
    // TODO
    return 0;
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

RNTupleWriterZeroMQ::RNTupleWriterZeroMQ(Config config) {
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
}

RNTupleWriterZeroMQ::~RNTupleWriterZeroMQ() {
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

    // TODO

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
