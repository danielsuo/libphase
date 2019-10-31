#pragma once

// Cycles per instruction phase detector
// Record CPI for each interval and determine new phase based on % variation
// from previous interval

#include "PhaseDetector.h"

namespace libphase {
class CPIPhaseDetector : public PhaseDetector {
  float prev_cpi_ = 0.0;

  public:
  CPIPhaseDetector(const YAML::Node& config)
  {
    name_ = "CPIPhaseDetector";
    init(config);
  }

  void
  intervalUpdate(
      const instruction& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) override
  {
    CPUCounters diff = curr_counters - prev_counters;
    float curr_cpi = ((float)diff.cycles) / ((float)diff.instructions);

    delta_ = fabs(curr_cpi - prev_cpi_) / prev_cpi_;

    log_ << curr_cpi << " " << prev_cpi_;

    prev_cpi_ = curr_cpi;
  }
};
}
