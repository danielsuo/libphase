#pragma once

#include "utils.h"

namespace libphase {
class TraceIO {
  bool isReader_;

  // NOTE: Using FILE* over fstream because Pin doesn't easily allow for writing
  // compressed files, so we cheat with popen
  bool ins_file_open_;
  bool bbl_file_open_;
  bool rtn_file_open_;

  bool start_of_trace_ = true;

  FILE* ins_file_;
  FILE* bbl_file_;
  FILE* rtn_file_;

  uint64_t num_ins_ = 0;
  uint64_t num_bbl_ = 0;
  uint64_t num_rtn_ = 0;

  uint64_t num_ins_curr_bbl_ = 0;
  uint64_t num_ins_curr_rtn_ = 0;

  std::string
  traceIOTypeString_() {
    return std::string("TraceIO") + std::string(" ") +
        std::string(isReader_ ? "Reader" : "Writer");
  }

 public:
  // NOTE: these should be private, but for convenience, we make them public for
  // now
  instruction curr_ins;
  basicblock curr_bbl;
  routine curr_rtn;

  basicblock next_bbl;
  routine next_rtn;

  TraceIO(
      bool isReader = true,
      std::string directory = "tmp",
      std::string ins_filename = "libphase.ins.gz",
      std::string bbl_filename = "libphase.bbl.gz",
      std::string rtn_filename = "libphase.rtn.gz")
      : isReader_(isReader),
        ins_file_open_(false),
        bbl_file_open_(false),
        rtn_file_open_(false) {
    std::cout << "Initializing " << traceIOTypeString_() << std::endl;

    std::string xz_command = std::string(isReader_ ? "xz -dc " : "xz -9 -c > ");
    const char* read_or_write = isReader_ ? "r" : "w";

    ins_file_ = popen(
        std::string(xz_command + directory + "/" + ins_filename).c_str(),
        read_or_write);
    bbl_file_ = popen(
        std::string(xz_command + directory + "/" + bbl_filename).c_str(),
        read_or_write);
    rtn_file_ = popen(
        std::string(xz_command + directory + "/" + rtn_filename).c_str(),
        read_or_write);

    if (!ins_file_) {
      std::cout << "Couldn't open ins file. Exiting." << std::endl;
      exit(1);
    }
    if (!bbl_file_) {
      std::cout << "Couldn't open bbl file. Exiting." << std::endl;
      exit(1);
    }
    if (!rtn_file_) {
      std::cout << "Couldn't open rtn file. Exiting." << std::endl;
      exit(1);
    }

    ins_file_open_ = true;
    bbl_file_open_ = true;
    rtn_file_open_ = true;
  }

  ~TraceIO() {
    if (ins_file_open_) {
      fclose(ins_file_);
      ins_file_open_ = false;
    }
    if (bbl_file_open_) {
      fclose(bbl_file_);
      bbl_file_open_ = false;
    }
    if (rtn_file_open_) {
      fclose(rtn_file_);
      rtn_file_open_ = false;
    }

    std::cout << traceIOTypeString_() << " destroyed" << std::endl;
  }

  size_t
  read() {
    size_t ret = read_ins();

    if (ret != 1) {
      return ret;
    }

    // If we're at the beginning of the trace
    if (start_of_trace_) {
      read_bbl();
      read_rtn();
      start_of_trace_ = false;
    }

    if (curr_ins.ip == next_bbl.address) {
      if (num_ins_ - num_ins_curr_bbl_ != curr_bbl.num_ins) {
        std::cout << "ERROR: invalid BBL read" << std::endl;
        std::cout << "New bbl: " << next_bbl.address << " "
                  << num_ins_ - num_ins_curr_bbl_ << " " << curr_bbl.num_ins
                  << std::endl;
        std::cout << "Valid: " << curr_bbl.valid << std::endl;
        std::cout << "Original: " << curr_bbl.original << std::endl;
        std::cout << "Fallthrough: " << curr_bbl.fallthrough << std::endl;
        std::cout << std::endl;
        // exit(1);
      }
      read_bbl();
    }

    if (curr_ins.ip == next_rtn.address) {
      if (next_rtn.id != curr_ins.routine_id) {
        std::cout << "ERROR: invalid RTN read" << std::endl;
        std::cout << "New rtn: " << next_rtn.address << " "
                  << num_ins_ - num_ins_curr_rtn_ << " " << curr_rtn.num_ins
                  << " " << curr_ins.opcode << " " << next_rtn.id << " "
                  << curr_ins.routine_id << std::endl;
        exit(1);
      }
      read_rtn();
    }

    return ret;
  }

  size_t
  read_ins() {
    if (!isReader_) {
      return 0;
    }

    size_t ret = fread(&curr_ins, sizeof(curr_ins), 1, ins_file_);

    if (ret == 1) {
      num_ins_++;
    }

    return ret;
  }

  size_t
  read_bbl() {
    if (!isReader_) {
      return 0;
    }

    curr_bbl = next_bbl;
    num_ins_curr_bbl_ = num_ins_;
    size_t ret = fread(&next_bbl, sizeof(next_bbl), 1, bbl_file_);

    if (ret == 1) {
      num_bbl_++;
    }

    return ret;
  }

  size_t
  read_rtn() {
    if (!isReader_) {
      return 0;
    }

    curr_rtn = next_rtn;
    num_ins_curr_rtn_ = num_ins_;
    size_t ret = fread(&next_rtn, sizeof(next_rtn), 1, rtn_file_);

    if (ret == 1) {
      num_rtn_++;
    }
  }

  size_t
  write_ins(instruction& ins) {
    if (isReader_) {
      return 0;
    }

    size_t ret = fwrite(&ins, sizeof(ins), 1, ins_file_);

    if (ret == 1) {
      num_ins_++;
    }

    return ret;
  }

  size_t
  write_bbl(basicblock& bbl) {
    if (isReader_) {
      return 0;
    }

    size_t ret = fwrite(&bbl, sizeof(bbl), 1, bbl_file_);

    if (ret == 1) {
      num_bbl_++;
    }

    return ret;
  }

  size_t
  write_rtn(routine& rtn) {
    if (isReader_) {
      return 0;
    }

    size_t ret = fwrite(&rtn, sizeof(rtn), 1, rtn_file_);

    if (ret == 1) {
      num_rtn_++;
    }

    return ret;
  }
};
} // namespace libphase
