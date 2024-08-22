// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RNTupleWriterZeroMQ.hxx"

#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageAllocator.hxx>
#include <ROOT/RPageSinkBuf.hxx>
#include <ROOT/RPageStorage.hxx>

#include <string>
#include <string_view>
#include <utility>

namespace {

using ROOT::Experimental::DescriptorId_t;
using ROOT::Experimental::NTupleSize_t;
using ROOT::Experimental::RException;
using ROOT::Experimental::RExtraTypeInfoDescriptor;
using ROOT::Experimental::RNTupleDescriptor;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::Internal::RColumn;
using ROOT::Experimental::Internal::RNTupleModelChangeset;
using ROOT::Experimental::Internal::RPage;
using ROOT::Experimental::Internal::RPageSink;

/// A page sink that sends all data to the RNTupleWriterZeroMQ server.
class RPageSinkZeroMQ : public RPageSink {
public:
  RPageSinkZeroMQ(const RNTupleWriterZeroMQ::Config &config)
      : RPageSink(config.fNTupleName, config.fOptions) {}

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
    // TODO or ignored?
  }
};

} // namespace

RNTupleWriterZeroMQ::RNTupleWriterZeroMQ(Config config) {}

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
