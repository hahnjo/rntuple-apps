// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef RNTupleWriterZeroMQ_hxx
#define RNTupleWriterZeroMQ_hxx

#include <ROOT/RNTupleMetrics.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>

#include <memory>
#include <string_view>

namespace ROOT {
class RNTupleModel;
class RNTupleWriter;
namespace Internal {
class RPageSink;
} // namespace Internal
} // namespace ROOT

/// Write RNTuple data from multiple clients using ZeroMQ
///
/// The server is represented by this class, while clients can be created with
/// CreateWorkerWriter. It is important that server and clients pass equivalent
/// Config objects with a matching RNTupleModel and RNTupleWriteOptions.
class RNTupleWriterZeroMQ {
public:
  struct Config {
    /// The model to write the ntuple.
    std::unique_ptr<ROOT::RNTupleModel> fModel;
    /// The ntuple name.
    std::string_view fNTupleName;
    /// Storage path for the ntuple.
    std::string_view fStorage;
    /// The ZeroMQ endpoint to create the server (see zmq_bind) and connect from
    /// the clients (see zmq_connect).
    std::string_view fEndpoint;
    /// Options for writing the ntuple.
    ROOT::RNTupleWriteOptions fOptions;
    /// Whether to send the payload data to the server. If not, clients only
    /// send the metadata and get back an offset to write the payload data
    /// themselves. This mode is more efficient, but requires that all processes
    /// can access the same file.
    bool fSendData = false;
    /// Whether to send the key to the clients. If yes, clients will write the
    /// key before writing the payload data. Only makes sense together with the
    /// previous option fSendData.
    bool fSendKey = false;
  };

private:
  /// The ZeroMQ context for this server.
  void *fContext;
  /// The ZeroMQ socket where clients can connect.
  void *fSocket;

  /// The persistent sink to write the ntuple.
  std::unique_ptr<ROOT::Internal::RPageSink> fSink;
  /// The model to write the ntuple; needs to be destructed before fSink.
  std::unique_ptr<ROOT::RNTupleModel> fModel;

  ROOT::Experimental::Detail::RNTupleMetrics fMetrics;
  /// Whether the clients are expected to send the payload data.
  bool fClientsSendData;
  /// Whether to send the key to the clients instead of writing.
  bool fSendKey;

  RNTupleWriterZeroMQ(Config config);

public:
  RNTupleWriterZeroMQ(const RNTupleWriterZeroMQ &) = delete;
  RNTupleWriterZeroMQ(RNTupleWriterZeroMQ &&) = default;
  RNTupleWriterZeroMQ &operator=(const RNTupleWriterZeroMQ &) = delete;
  RNTupleWriterZeroMQ &operator=(RNTupleWriterZeroMQ &&) = default;
  ~RNTupleWriterZeroMQ();

  /// Collect incoming data until the given number of clients terminate.
  void Collect(std::size_t clients);

  void EnableMetrics() { fMetrics.Enable(); }
  const ROOT::Experimental::Detail::RNTupleMetrics &GetMetrics() const {
    return fMetrics;
  }

  /// Recreate a new file and return a server object.
  static std::unique_ptr<RNTupleWriterZeroMQ> Recreate(Config config);

  /// Create a new RNTupleWriter that sends data to a server.
  static std::unique_ptr<ROOT::RNTupleWriter> CreateWorkerWriter(Config config);
};

#endif
