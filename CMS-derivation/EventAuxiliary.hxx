// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EventAuxiliary_hxx
#define EventAuxiliary_hxx

#include <string>

namespace edm {
// Put together from various CMSSW headers in DataFormats/Provenance/interface.

template <int I> struct Hash {
  typedef std::string value_type;
  value_type hash_;

  static short Class_Version() { return 11; }
};

enum HashedTypes {
  ModuleDescriptionType, // Obsolete
  ParameterSetType,
  ProcessHistoryType,
  ProcessConfigurationType,
  EntryDescriptionType, // Obsolete
  ParentageType
};

typedef Hash<ProcessHistoryType> ProcessHistoryID;

typedef unsigned long long EventNumber_t;
typedef unsigned int LuminosityBlockNumber_t;
typedef unsigned int RunNumber_t;

struct EventID {
  RunNumber_t run_;
  LuminosityBlockNumber_t luminosityBlock_;
  EventNumber_t event_;

  static short Class_Version() { return 12; }
};

struct Timestamp {
  unsigned int timeLow_;
  unsigned int timeHigh_;

  static short Class_Version() { return 10; }
};

struct EventAuxiliary {
  enum ExperimentType {
    Undefined = 0,
    PhysicsTrigger = 1,
    CalibrationTrigger = 2,
    RandomTrigger = 3,
    Reserved = 4,
    TracedEvent = 5,
    TestTrigger = 6,
    ErrorTrigger = 15
  };

  ProcessHistoryID processHistoryID_;
  EventID id_;
  std::string processGUID_;
  Timestamp time_;
  LuminosityBlockNumber_t luminosityBlock_;
  bool isRealData_;
  ExperimentType experimentType_;
  int bunchCrossing_;
  int orbitNumber_;
  int storeNumber_;

  static short Class_Version() { return 11; }
};

} // namespace edm

#endif
