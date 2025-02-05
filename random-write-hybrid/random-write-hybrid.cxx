// SPDX-License-Identifier: GPL-3.0-or-later

#include <Compression.h>
#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/TThreadExecutor.hxx>
#include <TFile.h>
#include <TROOT.h>

#include <mpi.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleParallelWriter;
using ROOT::Experimental::RNTupleWriteOptions;

int main(int argc, char *argv[]) {
  int provided = -1;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
  if (provided == MPI_THREAD_SINGLE) {
    fprintf(stderr, "MPI_Init_thread provided %d\n", provided);
    return 1;
  }

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  static constexpr int kRoot = 0;

  if (argc < 3) {
    fprintf(
        stderr,
        "Usage: ./random-write-hybrid dir entries <threads> <compression>\n");
    return 1;
  }

  const char *dir = argv[1];
  long entries = std::stol(argv[2]);
  int threads = 1;
  if (argc > 3) {
    threads = std::stoi(argv[3]);
  }
  int compression = ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose;
  if (argc > 4) {
    compression = std::stoi(argv[4]);
  }

  auto model = RNTupleModel::CreateBare();
  model->MakeField<std::uint64_t>("eventId");
  model->MakeField<std::vector<float>>("particles");

  RNTupleWriteOptions options;
  options.SetCompression(compression);
  options.SetMaxUnzippedPageSize(128 * 1024);

  if (rank == kRoot) {
    printf("ranks: %d, threads: %d, compression: %d\n\n", size, threads,
           compression);
  }

  std::mt19937 generator;
  std::poisson_distribution<int> poisson(5);
  std::uniform_real_distribution<float> uniform(0.0, 100.0);

  // We set the maximum unzipped page size to 128 * 1024; draw enough random
  // numbers to make sure we have enough for one page to avoid compression from
  // creating unrealistically small files. For simplicity, ignore the different
  // element type sizes (64 bit / 8 bytes for indices, 4 bytes for floats). Also
  // add a prime offset to avoid identical pages and prevent RNTuple from
  // same-page merging.
  size_t RandomNumbers = options.GetMaxUnzippedPageSize() + 13;
  std::vector<int> numParticlesV(RandomNumbers);
  std::vector<float> energiesV(RandomNumbers);
  for (size_t i = 0; i < RandomNumbers; i++) {
    numParticlesV[i] = poisson(generator);
    energiesV[i] = uniform(generator);
  }

  // Initialize ROOT outside of the measured section.
  ROOT::GetROOT();

  // Synchronize all ranks before starting the timer.
  MPI_Barrier(MPI_COMM_WORLD);

  auto start = std::chrono::steady_clock::now();

  std::string url(dir);
  url += "/random." + std::to_string(rank) + ".root";
  std::unique_ptr<TFile> file(TFile::Open(url.c_str(), "RECREATE"));
  auto writer =
      RNTupleParallelWriter::Append(std::move(model), "random", *file, options);
  writer->EnableMetrics();

  auto rankStart = std::chrono::steady_clock::now();

  // Use TThreadExecutor to start a function in parallel, all threads will write
  // the same data.
  ROOT::TThreadExecutor ex(threads);
  ex.Foreach(
      [&](unsigned int t) {
        auto fillContext = writer->CreateFillContext();
        fillContext->EnableMetrics();
        auto entry = fillContext->CreateEntry();

        auto eventId = entry->GetPtr<std::uint64_t>("eventId");
        auto particles = entry->GetPtr<std::vector<float>>("particles");

        int indexNumParticles = 0;
        int indexEnergies = 0;
        for (long i = 0; i < entries; i++) {
          *eventId = i + 1;

          int numParticles = numParticlesV[indexNumParticles];
          for (int j = 0; j < numParticles; j++) {
            particles->push_back(energiesV[indexEnergies]);
            indexEnergies = (indexEnergies + 1) % energiesV.size();
          }
          indexNumParticles = (indexNumParticles + 1) % numParticlesV.size();

          fillContext->Fill(*entry);

          particles->clear();
        }
        // In principle, it would not be needed to flush the cluster manually,
        // but this will include the last cluster in the metrics just below.
        fillContext->FlushCluster();
      },
      ROOT::TSeqU(threads));

  auto wallWrite =
      writer->GetMetrics()
          .GetCounter("RNTupleParallelWriter.RPageSinkFile.timeWallWrite")
          ->GetValueAsInt() /
      1e9;
  auto bytes =
      writer->GetMetrics()
          .GetCounter("RNTupleParallelWriter.RPageSinkFile.szWritePayload")
          ->GetValueAsInt();

  // Destruct the writer and commit the dataset.
  writer.reset();

  // Close the file and sync to permanent storage.
  file.reset();

  auto rankEnd = std::chrono::steady_clock::now();
  const std::chrono::duration<double> rankDuration = rankEnd - rankStart;

  auto bandwidthRank = bytes / 1e6 / rankDuration.count();
  printf("rank #%d: total: %f s, time writing: %f s, bandwidth: %f MB/s\n",
         rank, rankDuration.count(), wallWrite, bandwidthRank);

  // Synchronize the ranks to make sure all data is written.
  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == kRoot) {
    auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> duration = end - start;

    auto bytesTotal = size * bytes;
    auto bandwidthTotal = bytesTotal / 1e6 / duration.count();
    printf(" === total time: %f s ===\n", duration.count());
    printf(" === data volume: %f GB (%lu bytes) ===\n", bytesTotal / 1e9,
           bytesTotal);
    printf(" === bandwidth: %f MB/s ===\n", bandwidthTotal);
  }

  MPI_Finalize();

  return 0;
}
