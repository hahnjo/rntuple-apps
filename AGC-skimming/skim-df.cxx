// SPDX-License-Identifier: GPL-3.0-or-later

#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RResultHandle.hxx>
#include <ROOT/RSnapshotOptions.hxx>

#include "json.hpp"
using nlohmann::ordered_json;

#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

template <typename T>
static ROOT::RVec<T> filter(ROOT::RVec<T> &v, ROOT::RVec<int> &mask) {
  return v[mask];
}

static ROOT::RDF::RNode skim(ROOT::RDF::RNode &df) {
  static constexpr float Jet_Pt_Cut = 20 /* GeV */;
  auto filterJets = [](ROOT::RVec<float> &pt) { return pt > Jet_Pt_Cut; };
  static constexpr float Lepton_Pt_Cut = 20 /* GeV */;
  auto filterLeptons = [](ROOT::RVec<float> &pt) { return pt > Lepton_Pt_Cut; };

  auto goodElectrons =
      df.Define("goodElectrons", filterLeptons, {"Electron_pt"})
          .Redefine("Electron_cutBased", filter<std::int32_t>,
                    {"Electron_cutBased", "goodElectrons"})
          .Redefine("Electron_eta", filter<float>,
                    {"Electron_eta", "goodElectrons"})
          .Redefine("Electron_pt", filter<float>,
                    {"Electron_pt", "goodElectrons"})
          .Redefine("Electron_sip3d", filter<float>,
                    {"Electron_sip3d", "goodElectrons"});
  auto goodMuons =
      goodElectrons.Define("goodMuons", filterLeptons, {"Muon_pt"})
          .Redefine("Muon_eta", filter<float>, {"Muon_eta", "goodMuons"})
          .Redefine("Muon_pfRelIso04_all", filter<float>,
                    {"Muon_pfRelIso04_all", "goodMuons"})
          .Redefine("Muon_pt", filter<float>, {"Muon_pt", "goodMuons"})
          .Redefine("Muon_sip3d", filter<float>, {"Muon_sip3d", "goodMuons"})
          .Redefine("Muon_tightId", filter<bool>,
                    {"Muon_tightId", "goodMuons"});
  auto goodLeptons = goodMuons.Filter(
      [](ROOT::RVec<int> &goodElectrons, ROOT::RVec<int> &goodMuons) {
        return (Sum(goodElectrons) + Sum(goodMuons)) >= 1;
      },
      {"goodElectrons", "goodMuons"});
  auto goodJets =
      goodLeptons.Define("goodJets", filterJets, {"Jet_pt"})
          .Redefine("Jet_btagCSVV2", filter<float>,
                    {"Jet_btagCSVV2", "goodJets"})
          .Redefine("Jet_eta", filter<float>, {"Jet_eta", "goodJets"})
          .Redefine("Jet_jetId", filter<std::int32_t>,
                    {"Jet_jetId", "goodJets"})
          .Redefine("Jet_mass", filter<float>, {"Jet_mass", "goodJets"})
          .Redefine("Jet_phi", filter<float>, {"Jet_phi", "goodJets"})
          .Redefine("Jet_pt", filter<float>, {"Jet_pt", "goodJets"})
          .Filter([](ROOT::RVec<int> &goodJets) { return Sum(goodJets) >= 4; },
                  {"goodJets"});
  return goodJets;
}

static const ROOT::RDF::ColumnNames_t kSnapshotColumnNames = {
    // clang-format off
    "Electron_cutBased",
    "Electron_eta",
    "Electron_pt",
    "Electron_sip3d",

    "Jet_btagCSVV2",
    "Jet_eta",
    "Jet_jetId",
    "Jet_mass",
    "Jet_phi",
    "Jet_pt",

    "Muon_eta",
    "Muon_pfRelIso04_all",
    "Muon_pt",
    "Muon_sip3d",
    "Muon_tightId",
    // clang-format on
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./skim-df nanoaod_inputs.json <threads>\n");
    return 1;
  }

  const char *inputs_path = argv[1];
  int threads = 0;
  if (argc > 2) {
    threads = std::stoi(argv[2]);
  }

  if (threads >= 0) {
    ROOT::EnableImplicitMT(threads);
  }

  // Parse input structure and prepare (lazy) snapshots.
  std::ifstream f(inputs_path);
  auto inputs = ordered_json::parse(f);

  std::vector<ROOT::RDF::RResultHandle> handles;
  for (const auto &process : inputs.items()) {
    for (const auto &variation : process.value().items()) {
      std::vector<std::string> paths;
      for (const auto &file : variation.value()["files"]) {
        paths.push_back(file["path"].get<std::string>());
      }
      std::string outputFilename =
          process.key() + "." + variation.key() + ".root";

      ROOT::RDataFrame df("Events", paths);
      auto node = ROOT::RDF::AsRNode(df);
      node = skim(node);
      ROOT::RDF::RSnapshotOptions options;
      options.fCompressionAlgorithm =
          ROOT::RDF::RSnapshotOptions::ECAlgo::kZSTD;
      options.fCompressionLevel = 5;
      options.fLazy = true;
      options.fOutputFormat = ROOT::RDF::ESnapshotOutputFormat::kRNTuple;
      handles.push_back(node.Snapshot("Events", outputFilename,
                                      kSnapshotColumnNames, options));
    }
  }
  // Run all computation graphs in one go.
  ROOT::RDF::RunGraphs(handles);

  return 0;
}
