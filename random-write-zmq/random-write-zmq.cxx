// SPDX-License-Identifier: GPL-3.0-or-later

#include "RNTupleWriterZeroMQ.hxx"

#include <Compression.h>
#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <TROOT.h>

#include <chrono>
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
  if (argc < 3) {
    fprintf(stderr,
            "Usage: ./random-write-zmq entries procs <mode> <compression>\n");
    return 1;
  }

  long entries = std::atol(argv[1]);
  int procs = std::atoi(argv[2]);
  // mode = 0: sending all data to the writer
  // mode = 1: sending only metadata, payload written by individual processes
  int mode = 1;
  if (argc > 3) {
    mode = atoi(argv[3]);
  }
  int compression = ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose;
  if (argc > 4) {
    compression = atoi(argv[4]);
  }

  auto model = RNTupleModel::CreateBare();
  model->MakeField<unsigned long>("eventId");
  model->MakeField<std::vector<float>>("particles");

  static constexpr const char *Filename = "random.root";
  RNTupleWriterZeroMQ::Config config;
  config.fModel = std::move(model);
  config.fNTupleName = "random";
  config.fStorage = Filename;
  config.fEndpoint = "tcp://127.0.0.1:5555";
  config.fOptions.SetCompression(compression);
  config.fOptions.SetMaxUnzippedPageSize(128 * 1024);

  // Prepare the data, before forking.
  std::mt19937 generator;
  std::poisson_distribution<> poisson(5);
  std::uniform_real_distribution<> uniform(0.0, 100.0);

  // We set the default maximum unzipped page size to 128 * 1024; draw enough
  // random numbers to make sure we have enough for one page to avoid
  // compression from creating unrealistically small files. For simplicity,
  // ignore the different element type sizes (64 bit / 8 bytes for indices, 4
  // bytes for floats). Also add a prime offset to avoid identical pages and
  // prevent RNTuple from same-page merging.
  size_t RandomNumbers = config.fOptions.GetMaxUnzippedPageSize() + 13;
  std::vector<int> numParticlesV(RandomNumbers);
  std::vector<double> energiesV(RandomNumbers);
  for (size_t i = 0; i < RandomNumbers; i++) {
    numParticlesV[i] = poisson(generator);
    energiesV[i] = uniform(generator);
  }

  // Initialize ROOT outside of the measured section, before forking.
  ROOT::GetROOT();

  auto start = std::chrono::steady_clock::now();

  // Fork off worker processes.
  int procId;
  std::vector<pid_t> children;
  for (procId = 0; procId < procs; procId++) {
    pid_t pid = fork();
    if (pid == -1) {
      fprintf(stderr, "fork() failed\n");
      return 1;
    } else if (pid == 0) {
      // Children exit the loop.
      break;
    }
    children.push_back(pid);
  }

  if (procId == procs) {
    // The original main process. Start the server and collect data sent by the
    // workers.
    {
      auto writer = RNTupleWriterZeroMQ::Recreate(std::move(config));
      // TODO
    }

    for (auto &&pid : children) {
      int wstatus;
      if (waitpid(pid, &wstatus, 0) == -1) {
        fprintf(stderr, "waitpid() failed\n");
      } else if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != 0) {
        fprintf(stderr, "child did not exit successfully\n");
      }
    }

    // Sync to permanent storage.
    // CallFsync(Filename);

    auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> duration = end - start;

    printf(" === total time: %f s ===\n", duration.count());

    return 0;
  }

  {
    // A worker process. Connect to the server and start filling the RNTuple.
    auto writer = RNTupleWriterZeroMQ::CreateWorkerWriter(std::move(config));
    writer->EnableMetrics();

    auto entry = writer->CreateEntry();

    auto eventId = entry->GetPtr<unsigned long>("eventId");
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

      writer->Fill(*entry);

      particles->clear();
    }
    // In principle, it would not be needed to commit the cluster manually,
    // but this will include the last flush in the metrics just below.
    writer->CommitCluster();

    // Destruct the writer and commit the dataset.
    writer.reset();
  }

  return 0;
}
