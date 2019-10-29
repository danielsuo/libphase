#pragma once

#include "stdint.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace libphase {

struct GlobalCounters {
};

struct CPUCounters {
  // Accumulators: Counters for which we are interested in deltas from one
  // period to the next (e.g., cycles)
  uint64_t instructions = 0;
  uint64_t cycles = 0;

  // Resetable accumulators: Like accumulators, but reset to 0 each interval
  int64_t branches = 0;

  // Levels: Counters for which we are interested in the current level (e.g., LQ
  // occupancy)
  uint64_t lq_occupancy = 0;

  CPUCounters() {}

  friend CPUCounters
  operator-(const CPUCounters& c1, const CPUCounters& c2)
  {
    CPUCounters counters;

    // Accumulators
    counters.instructions = c1.instructions - c2.instructions;
    counters.cycles = c1.cycles - c2.cycles;

    // Resetable accumulators
    counters.branches = c1.branches - c2.branches;

    // Levels
    counters.lq_occupancy = c1.lq_occupancy;

    return counters;
  }

  void
  reset()
  {
    branches = 0;
  }
};

struct BRANCH {
  static const uint8_t branch = 0b00000001;
  static const uint8_t taken = 0b00000010;
  static const uint8_t call = 0b00000100;
  static const uint8_t direct = 0b00001000;
  static const uint8_t cond = 0b00010000;
  static const uint8_t fwd = 0b00100000;
  static const uint8_t ret = 0b01000000;

  static void
  print(uint8_t info)
  {
    std::cout << "Branch: ";
    if (info & branch) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Taken: ";
    if (info & taken) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Call: ";
    if (info & call) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Direct: ";
    if (info & direct) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Conditional: ";
    if (info & cond) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Forward: ";
    if (info & fwd) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }

    std::cout << "Return: ";
    if (info & ret) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }
  }
};

class input_instr {
  public:
  // instruction pointer or PC (Program Counter)
  uint64_t ip;

  // globally-unique routine id
  uint32_t routine_id;

  // Opcode: xed-iclass-enum.h
  // NOTE: ~1560 opcodes; there is room for more data here
  uint16_t opcode;

  // Category: xed-category-enum.h
  // NOTE: ~70 categories; there is room for more data here
  uint8_t category;

  // branch info
  uint8_t branch_info;

  // TODO: these shouldn't be hard-coded
  uint8_t destination_registers[2]; // output registers
  uint8_t source_registers[4];      // input registers

  uint64_t destination_memory[2]; // output memory
  uint64_t source_memory[4];      // input memory

  input_instr()
  {
    ip = 0;
    routine_id = 0;
    opcode = 0;
    category = 0;
    branch_info = 0;

    for (uint32_t i = 0; i < 4; i++) {
      source_registers[i] = 0;
      source_memory[i] = 0;
    }

    for (uint32_t i = 0; i < 2; i++) {
      destination_registers[i] = 0;
      destination_memory[i] = 0;
    }
  };
};
} // namespace libphase
