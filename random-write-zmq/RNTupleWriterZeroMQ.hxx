// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef RNTupleWriterZeroMQ_hxx
#define RNTupleWriterZeroMQ_hxx

#include <ROOT/RNTupleWriteOptions.hxx>

#include <memory>
#include <string_view>

namespace ROOT {
namespace Experimental {
class RNTupleModel;
class RNTupleWriter;
} // namespace Experimental
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
    std::unique_ptr<ROOT::Experimental::RNTupleModel> fModel;
    /// The ntuple name.
    std::string_view fNTupleName;
    /// Storage path for the ntuple.
    std::string_view fStorage;
    /// The ZeroMQ endpoint to create the server (see zmq_bind) and connect from
    /// the clients (see zmq_connect).
    std::string_view fEndpoint;
    /// Options for writing the ntuple.
    ROOT::Experimental::RNTupleWriteOptions fOptions;
  };

private:
  RNTupleWriterZeroMQ(Config config);

public:
  RNTupleWriterZeroMQ(const RNTupleWriterZeroMQ &) = delete;
  RNTupleWriterZeroMQ(RNTupleWriterZeroMQ &&) = default;
  RNTupleWriterZeroMQ &operator=(const RNTupleWriterZeroMQ &) = delete;
  RNTupleWriterZeroMQ &operator=(RNTupleWriterZeroMQ &&) = default;
  ~RNTupleWriterZeroMQ() = default;

  /// Recreate a new file and return a server object.
  static std::unique_ptr<RNTupleWriterZeroMQ> Recreate(Config config);

  /// Create a new RNTupleWriter that sends data to a server.
  static std::unique_ptr<ROOT::Experimental::RNTupleWriter>
  CreateWorkerWriter(Config config);
};

#endif
