// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MiniAODReader_hxx
#define MiniAODReader_hxx

#include "EventAuxiliary.hxx"

#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

static constexpr const char *kElectronsBranch =
    "patElectrons_slimmedElectrons__RECO.";
static constexpr const char *kJetsBranch = "patJets_slimmedJets__RECO.";
static constexpr const char *kJetsAK8Branch = "patJets_slimmedJetsAK8__RECO.";
static constexpr const char *kMuonsBranch = "patMuons_slimmedMuons__RECO.";
static constexpr const char *kPhotonsBranch =
    "patPhotons_slimmedPhotons__RECO.";
static constexpr const char *kTausBranch = "patTaus_slimmedTaus__RECO.";
static constexpr const char *kMETBranch = "patMETs_slimmedMETs__RECO.";

class PATObjectCollection {
  TBranch *fCollectionBranch;
  TBranch *fPtBranch;
  TBranch *fPhiBranch;
  TBranch *fEtaBranch;
  TBranch *fMBranch;
  TBranch *fQx3Branch = nullptr;
  TBranch *fPdgIdBranch = nullptr;

  int fNObjects = 0;
  std::vector<double> fPts;
  std::vector<double> fPhis;
  std::vector<double> fEtas;
  std::vector<double> fMs;
  std::vector<int> fPdgIds;
  std::vector<int> fQx3s;

  static TBranch *GetBranch(TTree &t, const std::string &b) {
    auto *branch = t.GetBranch(b.c_str());
    if (branch == nullptr) {
      throw std::runtime_error("could not find branch '" + b + "'");
    }
    branch->SetStatus(true);
    branch->SetMakeClass(true);
    return branch;
  }

public:
  PATObjectCollection(TTree &t, const std::string &branchName,
                      bool isJetsBranch = false) {
    fCollectionBranch = GetBranch(t, branchName + "obj");
    fCollectionBranch->SetAddress(&fNObjects);
    fPtBranch =
        GetBranch(t, branchName + "obj.m_state.p4Polar_.fCoordinates.fPt");
    fPhiBranch =
        GetBranch(t, branchName + "obj.m_state.p4Polar_.fCoordinates.fPhi");
    fEtaBranch =
        GetBranch(t, branchName + "obj.m_state.p4Polar_.fCoordinates.fEta");
    fMBranch =
        GetBranch(t, branchName + "obj.m_state.p4Polar_.fCoordinates.fM");
    if (!isJetsBranch) {
      fPdgIdBranch = GetBranch(t, branchName + "obj.m_state.pdgId_");
      fQx3Branch = GetBranch(t, branchName + "obj.m_state.qx3_");
    }
  }

  int GetNObjects() const { return fNObjects; }
  const std::vector<double> GetPts() const { return fPts; }
  const std::vector<double> GetPhis() const { return fPhis; }
  const std::vector<double> GetEtas() const { return fEtas; }
  const std::vector<double> GetMs() const { return fMs; }
  const std::vector<int> GetPdgIds() const { return fPdgIds; }
  const std::vector<int> GetQx3s() const { return fQx3s; }

  void GetEntry(Long64_t entry) {
    // fCollectionBranch is a TBranchElement, but we do not want it to iterate
    // over its subbranches - we will do this manually below for those that we
    // are interested in.
    fCollectionBranch->TBranch::GetEntry(entry);

    auto read = [&](auto &v, TBranch *b) {
      v.resize(fNObjects);
      // Set the branch addresses, which might have changed during the resize.
      b->SetAddress(v.data());
      b->GetEntry(entry);
    };

    read(fPts, fPtBranch);
    read(fPhis, fPhiBranch);
    read(fEtas, fEtaBranch);
    read(fMs, fMBranch);
    if (fPdgIdBranch != nullptr) {
      read(fPdgIds, fPdgIdBranch);
    }
    if (fQx3Branch != nullptr) {
      read(fQx3s, fQx3Branch);
    }
  }

  void Print(std::ostream &os) const {
    os << "{ nObjects: " << fNObjects;

    auto printV = [&](const auto &v, const char *l) {
      os << ", " << l << ": { ";
      for (int i = 0; i < fNObjects; i++) {
        if (i != 0) {
          os << ", ";
        }
        os << v[i];
      }
      os << " }";
    };

    printV(fPts, "pts");
    printV(fPhis, "phis");
    printV(fEtas, "etas");
    printV(fMs, "ms");
    if (fPdgIdBranch != nullptr) {
      printV(fPdgIds, "pdgIds");
    }
    if (fQx3Branch != nullptr) {
      printV(fQx3s, "Qx3s");
    }

    os << " }";
  }
};

class MET {
  TBranch *fCollectionBranch;
  TBranch *fPtBranch;
  TBranch *fPhiBranch;
  TBranch *fSumEtBranch;

  int fNObjects = 0;
  double fPt = 0;
  double fPhi = 0;
  double fSumEt = 0;

  static TBranch *SetupBranch(TTree &t, const char *suffix, void *addr) {
    std::string branchName = std::string(kMETBranch) + suffix;
    auto *branch = t.GetBranch(branchName.c_str());
    if (branch == nullptr) {
      throw std::runtime_error("could not find branch '" + branchName + "'");
    }
    branch->SetStatus(true);
    branch->SetMakeClass(true);
    branch->SetAddress(addr);
    return branch;
  }

public:
  MET(TTree &t) {
    fCollectionBranch = SetupBranch(t, "obj", &fNObjects);
    fPtBranch = SetupBranch(t, "obj.m_state.p4Polar_.fCoordinates.fPt", &fPt);
    fPhiBranch =
        SetupBranch(t, "obj.m_state.p4Polar_.fCoordinates.fPhi", &fPhi);
    fSumEtBranch = SetupBranch(t, "obj.sumet", &fSumEt);
  }

  double GetPt() const { return fPt; }
  double GetPhi() const { return fPhi; }
  double GetSumEt() const { return fSumEt; }

  void GetEntry(Long64_t entry) {
    // fCollectionBranch is a TBranchElement, but we do not want it to iterate
    // over its subbranches - we will do this manually below for those that we
    // are interested in.
    fCollectionBranch->TBranch::GetEntry(entry);
    if (fNObjects != 1) {
      throw std::runtime_error("not exactly one item in MET?!");
    }
    fPtBranch->GetEntry(entry);
    fPhiBranch->GetEntry(entry);
    fSumEtBranch->GetEntry(entry);
  }

  void Print(std::ostream &os) const {
    os << "{ pt: " << fPt << ", phi: " << fPhi << ", sumEt: " << fSumEt << " }";
  }
};

class MiniAODReader {
  std::unique_ptr<TFile> fFile;
  TTree *fEvents;

  Long64_t fEntry = -1;

  TBranch *fEventAuxiliaryBranch;
  edm::EventAuxiliary *fEventAuxiliary = nullptr;

  PATObjectCollection fElectrons;
  PATObjectCollection fJets;
  PATObjectCollection fJetsAK8;
  PATObjectCollection fMuons;
  PATObjectCollection fPhotons;
  PATObjectCollection fTaus;
  MET fMET;

  static TTree *OpenEvents(TFile &file) {
    auto *events = file.Get<TTree>("Events");
    if (events == nullptr) {
      throw std::runtime_error("could not find 'Events' tree");
    }
    events->SetBranchStatus("*", false);
    return events;
  }

public:
  MiniAODReader(std::unique_ptr<TFile> file)
      : fFile(std::move(file)), fEvents(OpenEvents(*fFile)),
        fElectrons(*fEvents, kElectronsBranch),
        fJets(*fEvents, kJetsBranch, /*isJetsBranch=*/true),
        fJetsAK8(*fEvents, kJetsAK8Branch, /*isJetsBranch=*/true),
        fMuons(*fEvents, kMuonsBranch), fPhotons(*fEvents, kPhotonsBranch),
        fTaus(*fEvents, kTausBranch), fMET(*fEvents) {
    fEventAuxiliaryBranch = fEvents->GetBranch("EventAuxiliary");
    if (fEventAuxiliaryBranch == nullptr) {
      throw std::runtime_error("could not find EventAuxiliary branch");
    }
    fEventAuxiliaryBranch->SetStatus(true);
    fEventAuxiliaryBranch->SetAddress(&fEventAuxiliary);
  }

  const edm::EventAuxiliary &GetEventAuxiliary() const {
    return *fEventAuxiliary;
  }
  const PATObjectCollection &GetElectrons() const { return fElectrons; }
  const PATObjectCollection &GetJets() const { return fJets; }
  const PATObjectCollection &GetJetsAK8() const { return fJetsAK8; }
  const PATObjectCollection &GetMuons() const { return fMuons; }
  const PATObjectCollection &GetPhotons() const { return fPhotons; }
  const PATObjectCollection &GetTaus() const { return fTaus; }
  const MET &GetMET() const { return fMET; }

  Long64_t GetNEntries() const { return fEvents->GetEntries(); }

  void GetEntry(Long64_t entry) {
    fEventAuxiliaryBranch->GetEntry(entry);
    fElectrons.GetEntry(entry);
    fJets.GetEntry(entry);
    fJetsAK8.GetEntry(entry);
    fMuons.GetEntry(entry);
    fPhotons.GetEntry(entry);
    fTaus.GetEntry(entry);
    fMET.GetEntry(entry);
    fEntry = entry;
  }

  void Print(std::ostream &os) const {
    if (fEntry == -1) {
      throw std::runtime_error("did not read an entry yet?!");
    }

    os << "Entry " << fEntry << " {\n";
    os << "  { run: " << fEventAuxiliary->id_.run_
       << ", luminosity block: " << fEventAuxiliary->id_.luminosityBlock_
       << ", event: " << fEventAuxiliary->id_.event_ << " }\n";
    os << "  electrons: ";
    fElectrons.Print(os);
    os << "\n";
    os << "  jets: ";
    fJets.Print(os);
    os << "\n";
    os << "  jetsAK8: ";
    fJetsAK8.Print(os);
    os << "\n";
    os << "  muons: ";
    fMuons.Print(os);
    os << "\n";
    os << "  photons: ";
    fPhotons.Print(os);
    os << "\n";
    os << "  taus: ";
    fTaus.Print(os);
    os << "\n";
    os << "  MET: ";
    fMET.Print(os);
    os << "\n";
    os << "}\n";
  }
};

#endif
