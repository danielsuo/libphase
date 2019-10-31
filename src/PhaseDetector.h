#pragma once

#include "utils.h"
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace libphase {
class PhaseDetector {
  protected:
  std::string name_ = "PhaseDetector";
  std::stringstream log_;

  YAML::Node config_;

  float threshold_ = 0.0;
  float delta_ = 0.0;

  public:
  void
  init(const YAML::Node& config)
  {
    config_ = config[name_];
    threshold_ = config_["threshold"].as<float>();
    std::cout << name_ << std::endl;
    std::cout << config_ << std::endl;
    std::cout << config_["threshold"] << std::endl;
  }

  void
  log()
  {
    std::cout << name_ << " " << delta_ << " " << log_.str() << std::endl;
    log_.str(std::string());
  }

  virtual void
  instructionUpdate(
      const instruction& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) {}

  virtual void
  intervalUpdate(
      const instruction& instr,
      const CPUCounters& curr_counters,
      const CPUCounters& prev_counters) {}
};
}
