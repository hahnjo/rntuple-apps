#ifndef Muon_hxx
#define Muon_hxx

#include <cstdint>
#include <vector>

struct MuonSegmentMatch {
  float x;
  float y;
};

struct MuonChamberMatch {
  float x;
  float y;
  std::vector<MuonSegmentMatch> segmentMatches;
};

struct Muon {
  float pt;
  float eta;
  float phi;
  float m;
  std::int8_t charge;
  std::vector<MuonChamberMatch> muMatches;
};

using Muons = std::vector<Muon>;

#endif
