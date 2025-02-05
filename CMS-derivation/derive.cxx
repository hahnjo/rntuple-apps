// SPDX-License-Identifier: GPL-3.0-or-later

#include "EventAuxiliary.hxx"
#include "MiniAODReader.hxx"
#include "NanoAODWriter.hxx"

#include <TError.h>
#include <TInterpreter.h>

#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/TThreadExecutor.hxx>

using ROOT::Experimental::RNTupleFillContext;
using ROOT::Experimental::RNTupleParallelWriter;

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

static void CallFsync(const char *filename) {
  int fd = open(filename, O_RDWR);
  if (fd < 0 || fsync(fd)) {
    abort();
  }
  close(fd);
}

static void ErrorHandler(int level, bool abort, const char *location,
                         const char *msg) {
  if (level == kWarning && strstr(msg, "no dictionary for class")) {
    return;
  }

  DefaultErrorHandler(level, abort, location, msg);
}

// Read MiniAOD input files, process, and fill NanoAOD output.
static void ProcessInput(const std::string &path, RNTupleFillContext &context) {
  std::unique_ptr<TFile> file(TFile::Open(path.c_str()));
  MiniAODReader reader(std::move(file));

  NanoAODEntry entry(context.CreateEntry());

  Long64_t nEntries = reader.GetNEntries();
  for (Long64_t i = 0; i < nEntries; i++) {
    reader.GetEntry(i);

    entry.run() = reader.GetEventAuxiliary().id_.run_;
    entry.luminosityBlock() = reader.GetEventAuxiliary().id_.luminosityBlock_;
    entry.event() = reader.GetEventAuxiliary().id_.event_;

    auto fillNanoAODCollection = [](NanoAODCollection &dst,
                                    const PATObjectCollection &src,
                                    bool isJet = false) {
      dst.pt().clear();
      dst.phi().clear();
      dst.eta().clear();
      dst.mass().clear();
      if (!isJet) {
        dst.pdgId().clear();
        dst.charge().clear();
      }

      for (double pt : src.GetPts()) {
        dst.pt().push_back(static_cast<float>(pt));
      }
      for (double phi : src.GetPhis()) {
        dst.phi().push_back(static_cast<float>(phi));
      }
      for (double eta : src.GetEtas()) {
        dst.eta().push_back(static_cast<float>(eta));
      }
      for (double m : src.GetMs()) {
        dst.mass().push_back(static_cast<float>(m));
      }
      if (!isJet) {
        dst.pdgId() = src.GetPdgIds();
        for (int qx3 : src.GetQx3s()) {
          dst.charge().push_back(qx3 / 3);
        }
      }
    };

    fillNanoAODCollection(entry.Electron(), reader.GetElectrons());
    fillNanoAODCollection(entry.FatJet(), reader.GetJetsAK8(), true);
    fillNanoAODCollection(entry.Jet(), reader.GetJets(), true);
    fillNanoAODCollection(entry.Muon(), reader.GetMuons());
    fillNanoAODCollection(entry.Photon(), reader.GetPhotons());
    fillNanoAODCollection(entry.Tau(), reader.GetTaus());

    entry.MET_pt() = reader.GetMET().GetPt();
    entry.MET_phi() = reader.GetMET().GetPhi();
    entry.MET_sumEt() = reader.GetMET().GetSumEt();

    context.Fill(entry.GetEntry());
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./derive file_index.txt <threads>\n");
    return 1;
  }

  const char *file_index_path = argv[1];
  int threads = 0;
  if (argc > 2) {
    threads = std::stoi(argv[2]);
  }

  SetErrorHandler(ErrorHandler);

  // Load the header at runtime so that we don't have to build dictionaries, but
  // all necessary information is available.
  if (!gInterpreter->Declare("#include \"EventAuxiliary.hxx\"")) {
    return 1;
  }

  // Read list of input files.
  std::ifstream file_index(file_index_path);
  std::vector<std::string> paths;
  while (true) {
    std::string path;
    std::getline(file_index, path);
    if (path.empty()) {
      break;
    }
    paths.push_back(path);
  }

  // Create NanoAOD writer.
  static constexpr const char *Filename = "nanoAOD.root";
  auto writer =
      RNTupleParallelWriter::Recreate(CreateModel(), "Events", Filename);

  ROOT::TThreadExecutor ex(threads);
  ex.Foreach(
      [&](unsigned int idx) {
        const auto &path = paths[idx];
        auto context = writer->CreateFillContext();
        ProcessInput(path, *context);
      },
      ROOT::TSeqU(paths.size()));

  writer.reset();

  CallFsync(Filename);

  return 0;
}
