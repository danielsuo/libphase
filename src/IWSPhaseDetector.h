#pragma once

// Instruction working set phase detector
// Compute normalized working set distance and identify new phase if distance is
// sufficiently large
// Distance = (Union - Intersection) / Union

#include "PhaseDetector.h"
#include <algorithm>
#include <set>

namespace libphase {
class IWSPhaseDetector : public PhaseDetector {
  std::set<uint64_t> ips1_;
  std::set<uint64_t> ips2_;
  bool ips1_is_prev_ = false;

  public:
  IWSPhaseDetector(const YAML::Node& config)
  {
    name_ = "IWSPhaseDetector";
    init(config);
  }

  void
  instructionUpdate(
      const input_instr& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) override
  {
    if (ips1_is_prev_) {
      ips2_.insert(instr.ip);
    } else {
      ips1_.insert(instr.ip);
    }
  }

  void
  intervalUpdate(
      const input_instr& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) override
  {
    std::set<uint64_t> iws_intersection;
    std::set<uint64_t> iws_union;
    std::set_intersection(
        ips1_.begin(),
        ips1_.end(),
        ips2_.begin(),
        ips2_.end(),
        std::inserter(iws_intersection, iws_intersection.begin()));
    std::set_union(
        ips1_.begin(),
        ips1_.end(),
        ips2_.begin(),
        ips2_.end(),
        std::inserter(iws_union, iws_union.begin()));

    delta_ = ((float)iws_union.size() - (float)iws_intersection.size()) / (float)iws_union.size();

    log_ << iws_intersection.size() << " " << iws_union.size();

    if (ips1_is_prev_) {
      ips1_.clear();
    } else {
      ips2_.clear();
    }

    ips1_is_prev_ = !ips1_is_prev_;
  }
};
}
