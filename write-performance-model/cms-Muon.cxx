// SPDX-License-Identifier: GPL-3.0-or-later

#include "Muon.hxx"

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
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

static constexpr std::size_t NumRepetitions = 10;
static constexpr std::size_t NumFields = 100;
static constexpr std::size_t NumEntries = 50000;

static constexpr double MeanNumMuons = 3.5;
static constexpr double MeanNumChamberMatches = 5.6;
static constexpr double MeanNumSegmentMatches = 0.37;

static double Run() {
  auto model = ROOT::RNTupleModel::CreateBare();
  for (std::size_t f = 0; f < NumFields; f++) {
    model->MakeField<Muons>("f" + std::to_string(f));
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

  // Prepare entry and resize vectors.
  auto entry = writer->CreateEntry();

  std::mt19937 generator;
  std::poisson_distribution<int> poissonMuons(MeanNumMuons);
  std::poisson_distribution<int> poissonChamberMatches(MeanNumChamberMatches);
  std::poisson_distribution<int> poissonSegmentMatches(MeanNumSegmentMatches);

  for (std::size_t f = 0; f < NumFields; f++) {
    auto ptr = entry->GetPtr<Muons>("f" + std::to_string(f));
    int numMuons = poissonMuons(generator);
    ptr->resize(numMuons);
    for (int muon = 0; muon < numMuons; muon++) {
      int numChamberMatches = poissonChamberMatches(generator);
      ptr->at(muon).muMatches.resize(numChamberMatches);
      for (int match = 0; match < numChamberMatches; match++) {
        int numSegmentMatches = poissonSegmentMatches(generator);
        ptr->at(muon).muMatches[match].segmentMatches.resize(numSegmentMatches);
      }
    }
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

int main(int argc, char *argv[]) {
  std::cout << "NumRepetitions: " << NumRepetitions
            << ", NumFields: " << NumFields << ", NumEntries: " << NumEntries
            << "\n";
  std::cout << "MeanNumMuons = " << MeanNumMuons
            << ", MeanNumChamberMatches = " << MeanNumChamberMatches
            << ", MeanNumSegmentMatches = " << MeanNumSegmentMatches << "\n\n";

  // Initialize ROOT before starting any benchmark.
  ROOT::GetROOT();

  gSystem->Load("./libMuon.so");

  std::cout << "Benchmarking..." << std::endl;

  double sum = 0, sum2 = 0;
  for (std::size_t r = 0; r < NumRepetitions; r++) {
    double timing = Run();
    std::cout << " " << timing << std::flush;
    sum += timing;
    sum2 += timing * timing;
  }
  double mean = sum / NumRepetitions;
  std::cout << "\n -> " << mean << " s";
  if (NumRepetitions > 1) {
    double var = (sum2 - sum * sum / NumRepetitions) / (NumRepetitions - 1);
    double stdev = std::sqrt(var);
    std::cout << " +- " << stdev << " s";
  }
  std::cout << "\n";

  return 0;
}
