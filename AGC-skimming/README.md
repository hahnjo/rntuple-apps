# Dataset Skimming of the Analysis Grand Challenge

1. Convert TTree data into RNTuple
2. Run this application to skim the dataset with basic filters from the AGC

## Retained fields

 * `Electron` collection: `cutBased`, `eta`, `pt`, `sip3d`
 * `Jet` collection: `btagCSVV2`, `eta`, `jetId`, `mass`, `phi`, `pt`
 * `Muon` collection: `eta`, `pfRelIso04_all`, `pt`, `sip3d`, `tightId`

## Applied Filters

These filters are supposed to be more coarse-grained than the AGC itself so that no events are lost and to model the case that analysts may want to change the cut values after the initial skimming.

 * Leptons (electrons and muons) must have $p_T > 20$ GeV (depending on the version, the AGC filters with a value of 25 or 30 GeV)
 * At least one lepton (the AGC requires exactly one lepton after applying all cuts)
 * Jets must have $p_T > 20$ GeV (depending on the version, the AGC filters with a value of 25 or 30 GeV)
 * At least four jets
