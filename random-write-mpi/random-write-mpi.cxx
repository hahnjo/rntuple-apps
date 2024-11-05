// SPDX-License-Identifier: GPL-3.0-or-later

#include "RNTupleWriterMPI.hxx"

#include <Compression.h>
#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <TROOT.h>

#include <mpi.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleWriteOptions;

static void CallFsync(const char *filename) {
  int fd = open(filename, O_RDWR);
  if (fd < 0 || fsync(fd)) {
    abort();
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int provided = -1;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
  if (provided != MPI_THREAD_MULTIPLE) {
    fprintf(stderr, "MPI_Init_thread provided %d\n", provided);
    return 1;
  }

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc < 2) {
    fprintf(stderr, "Usage: ./random-write-mpi entries <mode> <compression>\n");
    return 1;
  }

  long entries = std::atol(argv[1]);
  // mode = 0: sending all data to the writer
  // mode = 1: sending only metadata, payload written by individual processes
  // modes 2 and 3 employ the same implementation, but enable Direct I/O
  int mode = 1;
  if (argc > 2) {
    mode = atoi(argv[2]);
  }
  int compression = ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose;
  if (argc > 3) {
    compression = atoi(argv[3]);
  }

  auto model = RNTupleModel::CreateBare();
  model->MakeField<std::uint64_t>("eventId");
  model->MakeField<std::vector<float>>("particles");

  static constexpr const char *Filename = "random.root";
  RNTupleWriterMPI::Config config;
  config.fModel = std::move(model);
  config.fNTupleName = "random";
  config.fStorage = Filename;
  config.fOptions.SetCompression(compression);
  if (mode & 2) {
    config.fOptions.SetUseDirectIO(true);
  }
  config.fOptions.SetMaxUnzippedPageSize(128 * 1024);
  config.fSendData = !(mode & 1);

  // Prepare the data.
  std::mt19937 generator;
  std::poisson_distribution<> poisson(5);
  std::uniform_real_distribution<> uniform(0.0, 100.0);

  // We set the maximum unzipped page size to 128 * 1024; draw enough random
  // numbers to make sure we have enough for one page to avoid compression from
  // creating unrealistically small files. For simplicity, ignore the different
  // element type sizes (64 bit / 8 bytes for indices, 4 bytes for floats). Also
  // add a prime offset to avoid identical pages and  prevent RNTuple from
  // same-page merging.
  size_t RandomNumbers = config.fOptions.GetMaxUnzippedPageSize() + 13;
  std::vector<int> numParticlesV(RandomNumbers);
  std::vector<double> energiesV(RandomNumbers);
  for (size_t i = 0; i < RandomNumbers; i++) {
    numParticlesV[i] = poisson(generator);
    energiesV[i] = uniform(generator);
  }

  // Initialize ROOT outside of the measured section.
  ROOT::GetROOT();

  // Synchronize all ranks before starting the timer.
  MPI_Barrier(MPI_COMM_WORLD);

  auto start = std::chrono::steady_clock::now();

  // Create the writer and start filling the RNTuple.
  static constexpr int kRoot = 0;
  auto writer =
      RNTupleWriterMPI::Recreate(std::move(config), kRoot, MPI_COMM_WORLD);
  writer->EnableMetrics();

  auto entry = writer->CreateEntry();

  auto eventId = entry->GetPtr<std::uint64_t>("eventId");
  auto particles = entry->GetPtr<std::vector<float>>("particles");

  auto rankStart = std::chrono::steady_clock::now();

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

    writer->Fill(*entry);

    particles->clear();
  }
  // In principle, it would not be needed to commit the cluster manually,
  // but this will include the last flush in the metrics just below.
  writer->CommitCluster();

  auto rankEnd = std::chrono::steady_clock::now();
  const std::chrono::duration<double> rankDuration = rankEnd - rankStart;

  auto wallCS =
      writer->GetMetrics()
          .GetCounter("RNTupleWriter.RPageSinkBuf.timeWallCriticalSection")
          ->GetValueAsInt() /
      1e9;
  printf("rank #%d: total: %f s, in critical section: %f s, fraction c = %f\n",
         rank, rankDuration.count(), wallCS, wallCS / rankDuration.count());

  // Synchronize the ranks to make sure all data is written.
  MPI_Barrier(MPI_COMM_WORLD);

  double wallWrite;
  std::uint64_t bytes;
  if (rank == kRoot) {
    wallWrite = writer->GetMetrics()
                    .GetCounter("RNTupleWriter.RPageSinkBuf.RPageSinkMPI."
                                "Aggregator.timeWallWrite")
                    ->GetValueAsInt() /
                1e9;
    bytes = writer->GetMetrics()
                .GetCounter("RNTupleWriter.RPageSinkBuf.RPageSinkMPI."
                            "Aggregator.szWritePayload")
                ->GetValueAsInt();
  }

  // Destruct the writer and commit the dataset.
  writer.reset();

  if (rank == kRoot) {
    // Sync to permanent storage.
    CallFsync(Filename);

    auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> duration = end - start;

    auto bandwidthTotal = bytes / 1e6 / duration.count();
    if (config.fSendData) {
      printf(" === total time: %f s, time writing: %f s,"
             " average per process: %f s ===\n",
             duration.count(), wallWrite, wallWrite / size);
    } else {
      printf(" === total time: %f s, time writing (on aggregator): %f s ===\n",
             duration.count(), wallWrite);
    }
    printf(" === data volume: %f GB (%lu bytes) ===\n", bytes / 1e9, bytes);
    if (config.fSendData) {
      auto bandwidthWrite = bytes / 1e6 / wallWrite;
      printf(" === bandwidth: %f MB/s, of write time: %f MB/s ===\n",
             bandwidthTotal, bandwidthWrite);
    } else {
      printf(" === bandwidth: %f MB/s ===\n", bandwidthTotal);
    }
  }

  MPI_Finalize();

  return 0;
}