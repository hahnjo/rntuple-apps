// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RVec.hxx>
#include <ROOT/TThreadExecutor.hxx>

using ROOT::Experimental::RNTupleFillContext;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleParallelWriter;
using ROOT::Experimental::RNTupleReader;

#include "json.hpp"
using nlohmann::json;

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

static std::unique_ptr<RNTupleModel> CreateModel() {
  auto model = RNTupleModel::CreateBare();

  model->MakeField<ROOT::RVec<std::int32_t>>("Electron_cutBased");
  model->MakeField<ROOT::RVec<float>>("Electron_eta");
  model->MakeField<ROOT::RVec<float>>("Electron_pt");
  model->MakeField<ROOT::RVec<float>>("Electron_sip3d");

  model->MakeField<ROOT::RVec<float>>("Jet_btagCSVV2");
  model->MakeField<ROOT::RVec<float>>("Jet_eta");
  model->MakeField<ROOT::RVec<std::int32_t>>("Jet_jetId");
  model->MakeField<ROOT::RVec<float>>("Jet_mass");
  model->MakeField<ROOT::RVec<float>>("Jet_phi");
  model->MakeField<ROOT::RVec<float>>("Jet_pt");

  model->MakeField<ROOT::RVec<float>>("Muon_eta");
  model->MakeField<ROOT::RVec<float>>("Muon_pfRelIso04_all");
  model->MakeField<ROOT::RVec<float>>("Muon_pt");
  model->MakeField<ROOT::RVec<float>>("Muon_sip3d");
  model->MakeField<ROOT::RVec<bool>>("Muon_tightId");

  model->Freeze();

  return model;
}

static void ProcessInput(const std::string &path, RNTupleFillContext &context) {
  auto reader = RNTupleReader::Open(CreateModel(), "Events", path);
  auto readEntry = reader->GetModel().CreateEntry();

  // Get pointers to the read values.
  auto readElectron_cutBased =
      readEntry->GetPtr<ROOT::RVec<int>>("Electron_cutBased");
  auto readElectron_eta = readEntry->GetPtr<ROOT::RVec<float>>("Electron_eta");
  auto readElectron_pt = readEntry->GetPtr<ROOT::RVec<float>>("Electron_pt");
  auto readElectron_sip3d =
      readEntry->GetPtr<ROOT::RVec<float>>("Electron_sip3d");

  auto readJet_btagCSVV2 =
      readEntry->GetPtr<ROOT::RVec<float>>("Jet_btagCSVV2");
  auto readJet_eta = readEntry->GetPtr<ROOT::RVec<float>>("Jet_eta");
  auto readJet_jetId = readEntry->GetPtr<ROOT::RVec<int>>("Jet_jetId");
  auto readJet_mass = readEntry->GetPtr<ROOT::RVec<float>>("Jet_mass");
  auto readJet_phi = readEntry->GetPtr<ROOT::RVec<float>>("Jet_phi");
  auto readJet_pt = readEntry->GetPtr<ROOT::RVec<float>>("Jet_pt");

  auto readMuon_eta = readEntry->GetPtr<ROOT::RVec<float>>("Muon_eta");
  auto readMuon_pfRelIso04_all =
      readEntry->GetPtr<ROOT::RVec<float>>("Muon_pfRelIso04_all");
  auto readMuon_pt = readEntry->GetPtr<ROOT::RVec<float>>("Muon_pt");
  auto readMuon_sip3d = readEntry->GetPtr<ROOT::RVec<float>>("Muon_sip3d");
  auto readMuon_tightId = readEntry->GetPtr<ROOT::RVec<bool>>("Muon_tightId");

  // Get pointers to write values.
  auto writeEntry = context.CreateEntry();

  auto writeElectron_cutBased =
      writeEntry->GetPtr<ROOT::RVec<int>>("Electron_cutBased");
  auto writeElectron_eta =
      writeEntry->GetPtr<ROOT::RVec<float>>("Electron_eta");
  auto writeElectron_pt = writeEntry->GetPtr<ROOT::RVec<float>>("Electron_pt");
  auto writeElectron_sip3d =
      writeEntry->GetPtr<ROOT::RVec<float>>("Electron_sip3d");

  auto writeJet_btagCSVV2 =
      writeEntry->GetPtr<ROOT::RVec<float>>("Jet_btagCSVV2");
  auto writeJet_eta = writeEntry->GetPtr<ROOT::RVec<float>>("Jet_eta");
  auto writeJet_jetId = writeEntry->GetPtr<ROOT::RVec<int>>("Jet_jetId");
  auto writeJet_mass = writeEntry->GetPtr<ROOT::RVec<float>>("Jet_mass");
  auto writeJet_phi = writeEntry->GetPtr<ROOT::RVec<float>>("Jet_phi");
  auto writeJet_pt = writeEntry->GetPtr<ROOT::RVec<float>>("Jet_pt");

  auto writeMuon_eta = writeEntry->GetPtr<ROOT::RVec<float>>("Muon_eta");
  auto writeMuon_pfRelIso04_all =
      writeEntry->GetPtr<ROOT::RVec<float>>("Muon_pfRelIso04_all");
  auto writeMuon_pt = writeEntry->GetPtr<ROOT::RVec<float>>("Muon_pt");
  auto writeMuon_sip3d = writeEntry->GetPtr<ROOT::RVec<float>>("Muon_sip3d");
  auto writeMuon_tightId = writeEntry->GetPtr<ROOT::RVec<bool>>("Muon_tightId");

  static constexpr float Jet_Pt_Cut = 20 /* GeV */;
  static constexpr float Lepton_Pt_Cut = 20 /* GeV */;

  // Iterate over all entries in the input, drop leptons and jets according to
  // the pt cut, and entire entries if no lepton or less than four jets passed.
  for (auto numEntry : *reader) {
    reader->LoadEntry(numEntry, *readEntry);

    int leptons = 0;
    int jets = 0;

    writeElectron_cutBased->clear();
    writeElectron_eta->clear();
    writeElectron_pt->clear();
    writeElectron_sip3d->clear();
    for (size_t i = 0; i < readElectron_pt->size(); i++) {
      if ((*readElectron_pt)[i] < Lepton_Pt_Cut) {
        continue;
      }

      writeElectron_cutBased->push_back((*readElectron_cutBased)[i]);
      writeElectron_eta->push_back((*readElectron_eta)[i]);
      writeElectron_pt->push_back((*readElectron_pt)[i]);
      writeElectron_sip3d->push_back((*readElectron_sip3d)[i]);
      leptons++;
    }

    writeMuon_eta->clear();
    writeMuon_pfRelIso04_all->clear();
    writeMuon_pt->clear();
    writeMuon_sip3d->clear();
    writeMuon_tightId->clear();
    for (size_t i = 0; i < readMuon_pt->size(); i++) {
      if ((*readMuon_pt)[i] < Lepton_Pt_Cut) {
        continue;
      }

      writeMuon_eta->push_back((*readMuon_eta)[i]);
      writeMuon_pfRelIso04_all->push_back((*readMuon_pfRelIso04_all)[i]);
      writeMuon_pt->push_back((*readMuon_pt)[i]);
      writeMuon_sip3d->push_back((*readMuon_sip3d)[i]);
      writeMuon_tightId->push_back((*readMuon_tightId)[i]);
      leptons++;
    }
    if (leptons < 1) {
      continue;
    }

    writeJet_btagCSVV2->clear();
    writeJet_eta->clear();
    writeJet_jetId->clear();
    writeJet_mass->clear();
    writeJet_phi->clear();
    writeJet_pt->clear();
    for (size_t i = 0; i < readJet_pt->size(); i++) {
      if ((*readJet_pt)[i] < Jet_Pt_Cut) {
        continue;
      }

      writeJet_btagCSVV2->push_back((*readJet_btagCSVV2)[i]);
      writeJet_eta->push_back((*readJet_eta)[i]);
      writeJet_jetId->push_back((*readJet_jetId)[i]);
      writeJet_mass->push_back((*readJet_mass)[i]);
      writeJet_phi->push_back((*readJet_phi)[i]);
      writeJet_pt->push_back((*readJet_pt)[i]);
      jets++;
    }
    if (jets < 4) {
      continue;
    }

    context.Fill(*writeEntry);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./skim nanoaod_inputs.json <threads>\n");
    return 1;
  }

  const char *inputs_path = argv[1];
  int threads = 0;
  if (argc > 2) {
    threads = atoi(argv[2]);
  }

  std::ifstream f(inputs_path);
  json inputs = json::parse(f);

  for (const auto &process : inputs.items()) {
    for (const auto &variation : process.value().items()) {
      std::string filename = process.key() + "." + variation.key() + ".root";
      auto writer = RNTupleParallelWriter::Recreate(CreateModel(), "Events",
                                                    filename);

      std::vector<std::string> paths;
      for (const auto &file : variation.value()["files"]) {
        paths.push_back(file["path"].get<std::string>());
      }

      ROOT::TThreadExecutor ex(threads);
      ex.Foreach(
          [&](unsigned int idx) {
            const auto &path = paths[idx];
            auto context = writer->CreateFillContext();
            ProcessInput(path, *context);
          },
          ROOT::TSeqU(paths.size()));
    }
  }

  return 0;
}
