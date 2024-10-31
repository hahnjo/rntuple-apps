// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef RNTupleWriterMPI_hxx
#define RNTupleWriterMPI_hxx

#include <ROOT/RNTupleWriteOptions.hxx>

#include <mpi.h>

#include <memory>
#include <string_view>

namespace ROOT {
namespace Experimental {
class RNTupleModel;
class RNTupleWriter;
} // namespace Experimental
} // namespace ROOT

/// Write RNTuple data collectively from multiple processes using MPI.
class RNTupleWriterMPI {
public:
  struct Config {
    /// The model to write the ntuple.
    std::unique_ptr<ROOT::Experimental::RNTupleModel> fModel;
    /// The ntuple name.
    std::string_view fNTupleName;
    /// Storage path for the ntuple.
    std::string_view fStorage;
    /// Options for writing the ntuple.
    ROOT::Experimental::RNTupleWriteOptions fOptions;
    /// Whether to send the payload data via MPI. If not, processes only send
    /// the metadata and get back an offset to write the payload data
    /// themselves. This mode is more efficient, but requires that all processes
    /// can access the same file.
    bool fSendData = false;
  };

private:
  RNTupleWriterMPI() = delete;
  ~RNTupleWriterMPI() = delete;

public:
  /// Recreate a new file and return a new RNTupleWriter.
  ///
  /// This is a collective operation and all processes must pass the same
  /// arguments for comm and root.
  static std::unique_ptr<ROOT::Experimental::RNTupleWriter>
  Recreate(Config config, int root, MPI_Comm comm);
};

#endif
