// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NanoAODWriter_hxx
#define NanoAODWriter_hxx

#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleModel.hxx>

using ROOT::Experimental::REntry;
using ROOT::Experimental::RNTupleModel;

#include <memory>
#include <vector>

static std::unique_ptr<RNTupleModel> CreateModel() {
  auto model = RNTupleModel::CreateBare();

  model->MakeField<std::uint32_t>("run");
  model->MakeField<std::uint32_t>("luminosityBlock");
  model->MakeField<std::uint64_t>("event");

  for (std::string collection :
       {"Electron", "FatJet", "Jet", "Muon", "Photon", "Tau"}) {
    model->MakeField<std::vector<float>>(collection + "_pt");
    model->MakeField<std::vector<float>>(collection + "_phi");
    model->MakeField<std::vector<float>>(collection + "_eta");
    model->MakeField<std::vector<float>>(collection + "_mass");
    if (collection != "FatJet" && collection != "Jet") {
      model->MakeField<std::vector<std::int32_t>>(collection + "_pdgId");
      model->MakeField<std::vector<std::int32_t>>(collection + "_charge");
    }
  }

  model->MakeField<float>("MET_pt");
  model->MakeField<float>("MET_phi");
  model->MakeField<float>("MET_sumEt");

  return model;
}

class NanoAODCollection {
  std::shared_ptr<std::vector<float>> fPt;
  std::shared_ptr<std::vector<float>> fPhi;
  std::shared_ptr<std::vector<float>> fEta;
  std::shared_ptr<std::vector<float>> fMass;
  std::shared_ptr<std::vector<std::int32_t>> fPdgId;
  std::shared_ptr<std::vector<std::int32_t>> fCharge;

public:
  NanoAODCollection(const std::string &name, REntry &entry,
                    bool isJetCollection = false) {
    fPt = entry.GetPtr<std::vector<float>>(name + "_pt");
    fPhi = entry.GetPtr<std::vector<float>>(name + "_phi");
    fEta = entry.GetPtr<std::vector<float>>(name + "_eta");
    fMass = entry.GetPtr<std::vector<float>>(name + "_mass");
    if (!isJetCollection) {
      fPdgId = entry.GetPtr<std::vector<std::int32_t>>(name + "_pdgId");
      fCharge = entry.GetPtr<std::vector<std::int32_t>>(name + "_charge");
    }
  }

  std::vector<float> &pt() { return *fPt; }
  std::vector<float> &phi() { return *fPhi; }
  std::vector<float> &eta() { return *fEta; }
  std::vector<float> &mass() { return *fMass; }
  std::vector<std::int32_t> &pdgId() { return *fPdgId; }
  std::vector<std::int32_t> &charge() { return *fCharge; }
};

class NanoAODEntry {
  std::unique_ptr<REntry> fEntry;

  std::shared_ptr<std::uint32_t> fRun;
  std::shared_ptr<std::uint32_t> fLuminosityBlock;
  std::shared_ptr<std::uint64_t> fEvent;

  NanoAODCollection fElectron;
  NanoAODCollection fFatJet;
  NanoAODCollection fJet;
  NanoAODCollection fMuon;
  NanoAODCollection fPhoton;
  NanoAODCollection fTau;

  std::shared_ptr<float> fMET_pt;
  std::shared_ptr<float> fMET_phi;
  std::shared_ptr<float> fMET_sumEt;

public:
  NanoAODEntry(std::unique_ptr<REntry> entry)
      : fEntry(std::move(entry)), fElectron("Electron", *fEntry),
        fFatJet("FatJet", *fEntry, /*isJetCollection=*/true),
        fJet("Jet", *fEntry, /*isJetCollection=*/true), fMuon("Muon", *fEntry),
        fPhoton("Photon", *fEntry), fTau("Tau", *fEntry) {
    fRun = fEntry->GetPtr<std::uint32_t>("run");
    fLuminosityBlock = fEntry->GetPtr<std::uint32_t>("luminosityBlock");
    fEvent = fEntry->GetPtr<std::uint64_t>("event");
    fMET_pt = fEntry->GetPtr<float>("MET_pt");
    fMET_phi = fEntry->GetPtr<float>("MET_phi");
    fMET_sumEt = fEntry->GetPtr<float>("MET_sumEt");
  }

  REntry &GetEntry() { return *fEntry; }

  std::uint32_t &run() { return *fRun; }
  std::uint32_t &luminosityBlock() { return *fLuminosityBlock; }
  std::uint64_t &event() { return *fEvent; }
  NanoAODCollection &Electron() { return fElectron; }
  NanoAODCollection &FatJet() { return fFatJet; }
  NanoAODCollection &Jet() { return fJet; }
  NanoAODCollection &Muon() { return fMuon; }
  NanoAODCollection &Photon() { return fPhoton; }
  NanoAODCollection &Tau() { return fTau; }
  float &MET_pt() { return *fMET_pt; }
  float &MET_phi() { return *fMET_phi; }
  float &MET_sumEt() { return *fMET_sumEt; }
};

#endif
