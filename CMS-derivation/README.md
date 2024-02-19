# CMS Derivation: miniAOD to nanoAOD

1. Download miniAOD files from https://opendata.cern.ch/ (for example https://opendata.cern.ch/record/24117)
2. Run this application to produce nanoAOD-like RNTuples

## Contained fields

 * Event auxiliary information: `run`, `luminosityBlock`, `event`
 * `Electron`, `FatJet`, `Jet`, `Muon`, `Photon`, `Tau` collections:
   * `_pt`, `_phi`, `_eta`, `_mass`
   * except `FatJet` and `Jet`: `_pdgId`, `_charge`
 * `MET_pt`, `MET_phi`, `MET_sumEt`
