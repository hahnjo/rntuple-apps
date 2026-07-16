// SPDX-License-Identifier: GPL-3.0-or-later

#include "AoS-SoA.hxx"

#include <ROOT/RColumn.hxx>
#include <ROOT/RColumnElementBase.hxx>
#include <ROOT/RField.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleTypes.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageNullSink.hxx>
#include <TDictAttributeMap.h>
#include <TROOT.h>
#include <TSystem.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

static constexpr std::size_t NumRepetitions = 10;
static constexpr std::size_t NumFields = 100;
static constexpr std::size_t NumEntries = 100000;
static constexpr std::size_t MinElements = 1;
static constexpr std::size_t MaxElements = 13;

template <typename ValueType>
double Run(const std::string &typeName, std::function<void(ValueType &)> mod) {
  auto model = ROOT::RNTupleModel::CreateBare();
  for (std::size_t f = 0; f < NumFields; f++) {
    model->AddField(
        ROOT::RFieldBase::Create("f" + std::to_string(f), typeName).Unwrap());
  }

  // Create the writer.
  ROOT::RNTupleWriteOptions options;
  options.SetCompression(0);
  options.SetEnableSamePageMerging(false);
  options.SetEnablePageChecksums(false);

  auto sink = std::make_unique<ROOT::Experimental::Internal::RPageNullSink>(
      "null", options);
  auto writer =
      ROOT::Internal::CreateRNTupleWriter(std::move(model), std::move(sink));

  // Prepare entry.
  auto entry = writer->CreateEntry();

  for (std::size_t f = 0; f < NumFields; f++) {
    auto ptr = entry->GetPtr<ValueType>("f" + std::to_string(f));
    mod(*ptr);
  }

  // Fill entries, serializing the data.
  auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < NumEntries; i++) {
    writer->Fill(*entry);
  }

  // Destruct the writer and commit the dataset.
  writer.reset();

  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> duration = end - start;

  return duration.count();
}

template <typename ValueType>
void Benchmark(const std::string &typeName,
               std::function<void(ValueType &)> mod) {
  double sum = 0, sum2 = 0;
  std::cout << "   ";
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = Run<ValueType>(typeName, mod);
    std::cout << " " << timing << std::flush;
    sum += timing;
    sum2 += timing * timing;
  }
  double mean = sum / NumRepetitions;
  std::cout << "\n     -> " << mean << " s";
  if (NumRepetitions > 1) {
    double var = (sum2 - sum * sum / NumRepetitions) / (NumRepetitions - 1);
    double stdev = std::sqrt(var);
    std::cout << " +- " << stdev << " s";
  }
  std::cout << "\n";
}

int main(int argc, char *argv[]) {
  std::cout << "NumRepetitions: " << NumRepetitions
            << ", NumFields: " << NumFields << ", NumEntries: " << NumEntries
            << "\n\n";

  // Initialize ROOT before starting any benchmark.
  ROOT::GetROOT();

  gSystem->Load("./libAoS-SoA.so");

  std::cout << "Benchmarking AoS..." << std::endl;
  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](AoS &aos) { aos.resize(elements); };
    Benchmark<AoS>("AoS", mod);
  }
  std::cout << "\n";

  std::cout << "Benchmarking SoA..." << std::endl;
  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](SoA &soa) {
      soa.f1.resize(elements);
      soa.f2.resize(elements);
      soa.f3.resize(elements);
      soa.f4.resize(elements);
      soa.f5.resize(elements);
    };
    Benchmark<SoA>("SoA", mod);
  }
  std::cout << "\n";

  std::cout << "Benchmarking (experimental) native SoA..." << std::endl;

  // Mark the class as SoA with record type S
  auto cl = TClass::GetClass("SoA");
  cl->CreateAttributeMap();
  cl->GetAttributeMap()->AddProperty("rntuple.SoARecord", "S");
  {
    // Create one RSoAField, which will print the warning about the experimental
    // state.
    ROOT::Experimental::RSoAField f("f", "SoA");
  }

  for (std::size_t elements = MinElements; elements <= MaxElements;
       elements++) {
    std::cout << "  " << elements << " element(s):" << std::endl;
    auto mod = [elements](SoA &soa) {
      soa.f1.resize(elements);
      soa.f2.resize(elements);
      soa.f3.resize(elements);
      soa.f4.resize(elements);
      soa.f5.resize(elements);
    };
    Benchmark<SoA>("SoA", mod);
  }

  return 0;
}
