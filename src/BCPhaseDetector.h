#pragma once

// Branch counter phase detector
// Count number of dynamic branches per interval

#include "PhaseDetector.h"

namespace libphase {
class BCPhaseDetector : public PhaseDetector {
  public:
  BCPhaseDetector(const YAML::Node& config)
  {
    name_ = "BCPhaseDetector";
    init(config);
  }

  void
  intervalUpdate(
      const input_instr& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) override
  {
    int64_t diff = curr_counters.branches - prev_counters.branches;
    delta_ = fabs((float)diff / (float)prev_counters.branches);
    log_ << curr_counters.branches << " " << prev_counters.branches;
  }
};
}
