#pragma once

#include "PhaseDetector.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace libphase {
class PhaseManager {
  std::vector<PhaseDetector*> detectors_;
  uint64_t intervalStartInstruction_ = 0;

  // Settable parameters
  uint64_t intervalLength_ = 10000000;

  // Private functions
  bool isNewInterval(uint64_t current_cycle);

  public:
  PhaseManager(uint64_t intervalLength, const YAML::Node& config);

  bool updatePhaseDetectors(
      const input_instr& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters);
};
}
