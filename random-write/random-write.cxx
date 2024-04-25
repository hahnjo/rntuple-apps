// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/TThreadExecutor.hxx>
#include <TROOT.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleParallelWriter;
using ROOT::Experimental::RNTupleWriteOptions;

static void CallFsync(const char *filename) {
  int fd = open(filename, O_RDWR);
  if (fd < 0 || fsync(fd)) {
    abort();
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: ./random-write entries threads <mode>\n");
    return 1;
  }

  long entries = std::atol(argv[1]);
  int threads = std::atoi(argv[2]);
  // mode = 0: buffered RNTupleParallelWriter
  // mode = 1: RNTupleParallelWriter without RPageSinkBuf
  // (requires changes from https://github.com/root-project/root/pull/14939)
  // mode = 2: RNTupleParallelWriter without compression
  int mode = 0;
  if (argc > 3) {
    mode = atoi(argv[3]);
  }

  auto model = RNTupleModel::CreateBare();
  model->MakeField<unsigned long>("eventId");
  model->MakeField<std::vector<float>>("particles");

  RNTupleWriteOptions options;
  if (mode == 1) {
    options.SetUseBufferedWrite(false);
  } else if (mode == 2) {
    options.SetCompression(0);
  }

  std::mt19937 generator;
  std::poisson_distribution<> poisson(5);
  std::uniform_real_distribution<> uniform(0.0, 100.0);

  // The default target unzipped page size is 64 * 1024; draw enough random
  // numbers to make sure we have enough for one page to avoid compression from
  // creating unrealistically small files. For simplicity, ignore the different
  // element type sizes (64 bit / 8 bytes for indices, 4 bytes for floats).
  static constexpr size_t RandomNumbers = 64 * 1024;
  std::vector<int> numParticlesV(RandomNumbers);
  std::vector<double> energiesV(RandomNumbers);
  for (size_t i = 0; i < RandomNumbers; i++) {
    numParticlesV[i] = poisson(generator);
    energiesV[i] = uniform(generator);
  }

  // Initialize ROOT outside of the measured section.
  ROOT::GetROOT();

  auto start = std::chrono::steady_clock::now();

  static constexpr const char *Filename = "random.root";
  auto writer = RNTupleParallelWriter::Recreate(std::move(model), "random",
                                                Filename, options);
  writer->EnableMetrics();

  // Use TThreadExecutor to start a function in parallel, all threads will write
  // the same data.
  ROOT::TThreadExecutor ex(threads);
  ex.Foreach(
      [&](unsigned int t) {
        auto fillContext = writer->CreateFillContext();
        fillContext->EnableMetrics();
        auto entry = fillContext->CreateEntry();

        auto eventId = entry->GetPtr<unsigned long>("eventId");
        auto particles = entry->GetPtr<std::vector<float>>("particles");

        auto start = std::chrono::steady_clock::now();

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
        // In principle, it would not be needed to commit the cluster manually,
        // but this will include the last flush in the metrics just below.
        fillContext->CommitCluster();

        auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double> duration = end - start;

        const char *counter =
            "RNTupleFillContext.RPageSinkBuf.timeWallCriticalSection";
        if (!options.GetUseBufferedWrite()) {
          counter = "RNTupleFillContext.RPageUnbufferedSyncSink."
                    "timeWallCriticalSection";
        }
        auto wallCS =
            fillContext->GetMetrics().GetCounter(counter)->GetValueAsInt() /
            1e9;
        printf("thread #%d: total: %f s, in critical section: %f s, fraction c "
               "= %f\n",
               t, duration.count(), wallCS, wallCS / duration.count());
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

  // Sync to permanent storage.
  CallFsync(Filename);

  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> duration = end - start;

  auto bandwidthTotal = bytes / 1e6 / duration.count();
  auto bandwidthWrite = bytes / 1e6 / wallWrite;
  printf(" === total time: %f s, time writing: %f s, average per thread: %f s "
         "===\n",
         duration.count(), wallWrite, wallWrite / threads);
  printf(" === data volume: %f GB (%lu bytes) ===\n", bytes / 1e9, bytes);
  printf(" === bandwidth: %f MB/s, of write time: %f MB/s ===\n",
         bandwidthTotal, bandwidthWrite);

  return 0;
}
