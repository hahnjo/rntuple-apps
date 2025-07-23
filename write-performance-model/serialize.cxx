// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/REntry.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RPageNullSink.hxx>
#include <TROOT.h>
#include <TSystem.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>

static constexpr std::size_t NumRepetitions = 10;

static double ReadAndSerialize(const char *ntupleName, const char *storage,
                               bool serialize = true) {
  auto start = std::chrono::steady_clock::now();

  // Create reader and get model.
  ROOT::RNTupleDescriptor::RCreateModelOptions modelOptions;
  modelOptions.SetReconstructProjections(true);
  auto reader = ROOT::RNTupleReader::Open(modelOptions, ntupleName, storage);
  auto model = reader->GetModel().Clone();

  // Create writer to only serialize the data.
  ROOT::RNTupleWriteOptions options;
  options.SetCompression(0);
  options.SetEnableSamePageMerging(false);
  options.SetEnablePageChecksums(false);
  auto sink = std::make_unique<ROOT::Experimental::Internal::RPageNullSink>(
      ntupleName, options);
  auto writer =
      ROOT::Internal::CreateRNTupleWriter(std::move(model), std::move(sink));

  // Create entries and link their shared_ptr's.
  auto readerEntry = reader->CreateEntry();
  auto writerEntry = writer->GetModel().CreateBareEntry();
  for (const auto &value : *readerEntry) {
    writerEntry->BindValue(value.GetField().GetFieldName(),
                           value.GetPtr<void>());
  }

  // Iterate over all entries, reading and writing.
  for (auto index : *reader) {
    reader->LoadEntry(index, *readerEntry);
    if (serialize) {
      writer->Fill(*writerEntry);
    }
  }

  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> duration = end - start;

  return duration.count();
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: ./serialize ntupleName storage" << std::endl;
    return 1;
  }

  const char *ntupleName = argv[1];
  const char *storage = argv[2];

  // Initialize ROOT before starting any benchmark.
  ROOT::GetROOT();

  gSystem->Load("./libPHYSLITE.so");

  // Run read + serialization once to warm up the system.
  ReadAndSerialize(ntupleName, storage);

  std::cout << "Reading \"" << ntupleName << "\"..." << std::endl;
  double sumRead = 0, sum2Read = 0;
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = ReadAndSerialize(ntupleName, storage, /*serialize=*/false);
    std::cout << " " << timing << std::flush;
    sumRead += timing;
    sum2Read += timing * timing;
  }
  double meanRead = sumRead / NumRepetitions;
  double varRead =
      (sum2Read - sumRead * sumRead / NumRepetitions) / (NumRepetitions - 1);
  double stdevRead = std::sqrt(varRead);
  std::cout << "\n -> mean: " << meanRead << " s +- " << stdevRead << " s\n\n";

  std::cout << "Reading and serializing..." << std::endl;
  double sumReadAndSerialize = 0, sum2ReadAndSerialize = 0;
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = ReadAndSerialize(ntupleName, storage, /*serialize=*/true);
    std::cout << " " << timing << std::flush;
    sumReadAndSerialize += timing;
    sum2ReadAndSerialize += timing * timing;
  }
  double meanReadAndSerialize = sumReadAndSerialize / NumRepetitions;
  double varReadAndSerialize =
      (sum2ReadAndSerialize -
       sumReadAndSerialize * sumReadAndSerialize / NumRepetitions) /
      (NumRepetitions - 1);
  double stdevReadAndSerialize = std::sqrt(varReadAndSerialize);
  std::cout << "\n -> mean: " << meanReadAndSerialize << " s +- "
            << stdevReadAndSerialize << " s\n\n";

  double meanSerialize = meanReadAndSerialize - meanRead;
  double errorSerialize = std::sqrt(varRead + varReadAndSerialize);
  std::cout << "serialization: " << meanSerialize << " s +- " << errorSerialize
            << " s" << std::endl;

  return 0;
}
