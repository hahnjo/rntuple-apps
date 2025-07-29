// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/RField.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageNullSink.hxx>
#include <TROOT.h>
#include <TSystem.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

static constexpr std::size_t NumRepetitions = 10;
static constexpr std::size_t NumFields = 1000;
static constexpr std::size_t NumEntries = 1000000;

static double RunBenchmark(const ROOT::RFieldBase &proto,
                           std::function<void(void *)> mod = {}) {
  // Wrap proto field in record field to "throw off" branch prediction a bit.
  std::vector<std::unique_ptr<ROOT::RFieldBase>> itemFields;
  itemFields.push_back(proto.Clone("_0"));
  const ROOT::RRecordField recordField("record", std::move(itemFields));

  // Create dummy fields to "throw off" branch prediction a bit more.
  std::vector<std::unique_ptr<ROOT::RFieldBase>> emptyItemFields;
  const ROOT::RRecordField empty("empty", std::move(emptyItemFields));

  std::vector<std::unique_ptr<ROOT::RFieldBase>> dummyItemFields;
  dummyItemFields.push_back(empty.Clone("_0"));
  dummyItemFields.push_back(empty.Clone("_1"));
  dummyItemFields.push_back(empty.Clone("_2"));
  const ROOT::RRecordField dummy("dummy", std::move(dummyItemFields));

  auto model = ROOT::RNTupleModel::CreateBare();
  for (std::size_t f = 0; f < NumFields; f++) {
    model->AddField(dummy.Clone("d" + std::to_string(f)));
    model->AddField(recordField.Clone("f" + std::to_string(f)));
  }

  ROOT::RNTupleWriteOptions options;
  options.SetCompression(0);
  options.SetEnableSamePageMerging(false);
  options.SetEnablePageChecksums(false);

  // Start timing & create writer.
  auto start = std::chrono::steady_clock::now();

  auto sink = std::make_unique<ROOT::Experimental::Internal::RPageNullSink>(
      "null", options);
  auto writer =
      ROOT::Internal::CreateRNTupleWriter(std::move(model), std::move(sink));

  // Prepare entry, potentially calling modification function.
  auto entry = writer->CreateEntry();
  if (mod) {
    for (std::size_t f = 0; f < NumFields; f++) {
      mod(entry->GetPtr<void>("f" + std::to_string(f)).get());
    }
  }

  // Fill entries
  for (std::size_t i = 0; i < NumEntries; i++) {
    writer->Fill(*entry);
  }

  // Destruct the writer and commit the dataset.
  writer.reset();

  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> duration = end - start;

  return duration.count();
}

struct BenchmarkResult {
  std::vector<double> timings;
  double mean = 0;
  double var = 0;
  double stdev = 0;
};

static BenchmarkResult Benchmark(const ROOT::RFieldBase &proto,
                                 std::function<void(void *)> mod = {}) {
  BenchmarkResult result;
  double sum = 0, sum2 = 0;
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = RunBenchmark(proto, mod);
    result.timings.push_back(timing);
    sum += timing;
    sum2 += timing * timing;
  }

  result.mean = sum / NumRepetitions;
  if (NumRepetitions > 1) {
    result.var = (sum2 - sum * sum / NumRepetitions) / (NumRepetitions - 1);
    result.stdev = std::sqrt(result.var);
  }

  return result;
}

static double CombineErrors(const BenchmarkResult &r1,
                            const BenchmarkResult &r2) {
  return std::sqrt(r1.var + r2.var);
}

static void PrintTimings(const BenchmarkResult &result) {
  for (auto timing : result.timings) {
    std::cout << " " << timing;
  }
  std::cout << "\n";
}

int main(int argc, char *argv[]) {
  std::cout << "NumRepetitions: " << NumRepetitions
            << ", NumFields: " << NumFields << ", NumEntries: " << NumEntries
            << "\n\n";

  // Initialize ROOT before starting any benchmark.
  ROOT::GetROOT();

  std::vector<std::pair<std::string, BenchmarkResult>> results;
  auto benchmark = [&results](std::string_view label,
                              const ROOT::RFieldBase &proto,
                              std::function<void(void *)> mod = {}) {
    std::cout << "Benchmarking " << label << " ...\n";
    std::cout << std::flush;
    auto result = Benchmark(proto, mod);
    results.emplace_back(label, result);
    PrintTimings(result);
    std::cout << std::flush;
    return result;
  };

  BenchmarkResult int32 =
      benchmark("std::int32_t", ROOT::RField<std::int32_t>("int32"));
  BenchmarkResult int64 =
      benchmark("std::int64_t", ROOT::RField<std::int64_t>("int64"));

  BenchmarkResult tuple_int32;
  {
    ROOT::RField<std::tuple<std::int32_t>> f("tuple_int32");
    tuple_int32 = benchmark("std::tuple<std::int32_t>", f);
  }
  BenchmarkResult tuple_int64;
  {
    ROOT::RField<std::tuple<std::int64_t>> f("tuple_int64");
    tuple_int64 = benchmark("std::tuple<std::int64_t>", f);
  }

  BenchmarkResult vector_int32_0;
  {
    ROOT::RField<std::vector<std::int32_t>> f("vector_int32");
    vector_int32_0 = benchmark("std::vector<std::int32_t> with 0 elements", f);
  }

  BenchmarkResult vector_tuple_int32_2;
  {
    using FieldType = std::vector<std::tuple<std::int32_t>>;
    ROOT::RField<FieldType> f("vector_tuple_int32");
    auto mod = [](void *ptr) { static_cast<FieldType *>(ptr)->resize(2); };
    vector_tuple_int32_2 = benchmark(
        "std::vector<std::tuple<std::int32_t>> with 2 elements", f, mod);
  }
  BenchmarkResult vector_tuple_int32_int32;
  {
    using FieldType = std::vector<std::tuple<std::int32_t, std::int32_t>>;
    ROOT::RField<FieldType> f("vector_tuple_int32_int32");
    auto mod = [](void *ptr) { static_cast<FieldType *>(ptr)->resize(1); };
    vector_tuple_int32_int32 = benchmark(
        "std::vector<std::tuple<std::int32_t, std::int32_t>> with 1 element", f,
        mod);
  }

  BenchmarkResult tuple_vector_tuple_int32_2;
  {
    using FieldType = std::vector<std::tuple<std::int32_t>>;
    ROOT::RField<std::tuple<FieldType>> f("tuple_vector_tuple_int32");
    auto mod = [](void *ptr) { static_cast<FieldType *>(ptr)->resize(2); };
    tuple_vector_tuple_int32_2 = benchmark(
        "std::tuple<std::vector<std::tuple<std::int32_t>>> with 2 elements", f,
        mod);
  }
  BenchmarkResult vector_tuple_tuple_int32_2;
  {
    using FieldType = std::vector<std::tuple<std::tuple<std::int32_t>>>;
    ROOT::RField<FieldType> f("vector_tuple_tuple_int32");
    auto mod = [](void *ptr) { static_cast<FieldType *>(ptr)->resize(2); };
    vector_tuple_tuple_int32_2 = benchmark(
        "std::vector<std::tuple<std::tuple<std::int32_t>>> with 2 elements", f,
        mod);
  }

  BenchmarkResult vector_tuple_int64_1;
  {
    using FieldType = std::vector<std::tuple<std::int64_t>>;
    ROOT::RField<FieldType> f("vector_tuple_int64");
    auto mod = [](void *ptr) { static_cast<FieldType *>(ptr)->resize(1); };
    vector_tuple_int64_1 = benchmark(
        "std::vector<std::tuple<std::int64_t>> with 1 element", f, mod);
  }

  std::cout << "\n === SUMMARY ===\n";
  for (auto &&result : results) {
    std::cout << "  " << result.first << ": " << result.second.mean << " s +- "
              << result.second.stdev << " s\n";
  }

  std::cout << "\n === RESULTS ===\n";
  static constexpr double ToUsPerFieldPerEntry = 1e6 / NumFields / NumEntries;
  double perByte = (int64.mean - int32.mean) / 4 * ToUsPerFieldPerEntry;
  double perByteError = CombineErrors(int64, int32) / 4 * ToUsPerFieldPerEntry;
  std::cout << "  per byte: " << perByte << " us +- " << perByteError
            << " us\n";

  double perRecord = (tuple_int32.mean - int32.mean) * ToUsPerFieldPerEntry;
  double perRecordError =
      CombineErrors(tuple_int32, int32) * ToUsPerFieldPerEntry;
  double perRecord64 = (tuple_int64.mean - int64.mean) * ToUsPerFieldPerEntry;
  double perRecord64Error =
      CombineErrors(tuple_int64, int64) * ToUsPerFieldPerEntry;
  double perRecordVector =
      (tuple_vector_tuple_int32_2.mean - vector_tuple_int32_2.mean) *
      ToUsPerFieldPerEntry;
  double perRecordVectorError =
      CombineErrors(tuple_vector_tuple_int32_2, vector_tuple_int32_2) *
      ToUsPerFieldPerEntry;
  std::cout << "  per record field: " << perRecord << " us +- "
            << perRecordError << " us\n";
  std::cout << "    from std::int64_t: " << perRecord64 << " us +- "
            << perRecord64Error << " us\n";
  std::cout << "    from std::vector<std::tuple<std::int32_t>>: "
            << perRecordVector << " us +- " << perRecordVectorError << " us\n";

  double perVector = (vector_int32_0.mean - int64.mean) * ToUsPerFieldPerEntry;
  double perVectorError =
      CombineErrors(vector_int32_0, int64) * ToUsPerFieldPerEntry;
  std::cout << "  per vector field: " << perVector << " us +- "
            << perVectorError << " us\n";

  double perColumn =
      (vector_tuple_int32_int32.mean - vector_tuple_int32_2.mean) *
      ToUsPerFieldPerEntry;
  double perColumnError =
      CombineErrors(vector_tuple_int32_int32, vector_tuple_int32_2) *
      ToUsPerFieldPerEntry;
  std::cout << "  per column: " << perColumn << " us +- " << perColumnError
            << " us\n";

  double perRecordAppend =
      (vector_tuple_tuple_int32_2.mean - tuple_vector_tuple_int32_2.mean) *
      ToUsPerFieldPerEntry;
  double perRecordAppendError =
      CombineErrors(vector_tuple_tuple_int32_2, tuple_vector_tuple_int32_2) *
      ToUsPerFieldPerEntry;
  std::cout << "  per record field append: " << perRecordAppend << " us +- "
            << perRecordAppendError << " us\n";

  double perColumnAppend =
      (vector_tuple_int32_2.mean - vector_tuple_int64_1.mean) *
      ToUsPerFieldPerEntry;
  double perColumnAppendError =
      CombineErrors(vector_tuple_int32_2, vector_tuple_int64_1) *
      ToUsPerFieldPerEntry;
  std::cout << "  per column append: " << perColumnAppend << " us +- "
            << perColumnAppendError << " us\n";

  return 0;
}
