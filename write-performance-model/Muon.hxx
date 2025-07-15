#ifndef Muon_hxx
#define Muon_hxx

#include <array>
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

struct Candidate {};

struct PolarLorentzVector {
  float pt;
  float eta;
  float phi;
  float m;
};

struct ParticleState {
  PolarLorentzVector p4Polar;
  std::int8_t charge;
};

struct LeafCandidate : public Candidate {
  ParticleState state;
};

struct RecoCandidate : public LeafCandidate {};

struct RecoMuon : public RecoCandidate {
  std::vector<MuonChamberMatch> muMatches;
};

template <class ObjectType> struct PATObject : public ObjectType {};

template <class ObjectType> struct PATLepton : public PATObject<ObjectType> {};

struct PATMuon : public PATLepton<RecoMuon> {};

using PATMuons = std::vector<PATMuon>;

struct ArrayMuonSegmentMatch {
  std::array<float, 2> xy;
};

struct ArrayMuonChamberMatch {
  std::array<float, 2> xy;
  std::vector<ArrayMuonSegmentMatch> segmentMatches;
};

struct ArrayMuon {
  std::array<float, 4> p4;
  std::int8_t charge;
  std::vector<ArrayMuonChamberMatch> muMatches;
};

using ArrayMuons = std::vector<ArrayMuon>;

#endif
