// SPDX-License-Identifier: GPL-3.0-or-later

#include "AoS-SoA.hxx"

#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageNullSink.hxx>
#include <TROOT.h>
#include <TSystem.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <utility>

static constexpr std::size_t NumRepetitions = 10;
static constexpr std::size_t NumFields = 100;
static constexpr std::size_t NumEntries = 100000;
static constexpr std::size_t MinElements = 1;
static constexpr std::size_t MaxElements = 13;

template <typename FieldType> double Run(std::function<void(FieldType &)> mod) {
  auto model = ROOT::RNTupleModel::CreateBare();
  for (std::size_t f = 0; f < NumFields; f++) {
    model->MakeField<FieldType>("f" + std::to_string(f));
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
    auto ptr = entry->GetPtr<FieldType>("f" + std::to_string(f));
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

template <typename FieldType>
void Benchmark(std::function<void(FieldType &)> mod) {
  double sum = 0, sum2 = 0;
  std::cout << "   ";
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = Run<FieldType>(mod);
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
    Benchmark<AoS>(mod);
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
    Benchmark<SoA>(mod);
  }

  return 0;
}
