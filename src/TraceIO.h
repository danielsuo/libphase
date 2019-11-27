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

  FILE* ins_file_;
  FILE* bbl_file_;
  FILE* rtn_file_;

  std::string
  traceIOTypeString_() {
    return std::string("TraceIO") + std::string(" ") +
        std::string(isReader_ ? "Reader" : "Writer");
  }

 public:
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

    if (isReader_) {
      ins_file_ = popen(
          std::string("xz -dc " + directory + "/" + ins_filename).c_str(), "w");
      bbl_file_ = popen(
          std::string("xz -dc " + directory + "/" + bbl_filename).c_str(), "w");
      rtn_file_ = popen(
          std::string("xz -dc " + directory + "/" + rtn_filename).c_str(), "w");
    } else {
      ins_file_ = popen(
          std::string("xz -9 -c > " + directory + "/" + ins_filename).c_str(),
          "w");
      bbl_file_ = popen(
          std::string("xz -9 -c > " + directory + "/" + bbl_filename).c_str(),
          "w");
      rtn_file_ = popen(
          std::string("xz -9 -c > " + directory + "/" + rtn_filename).c_str(),
          "w");
    }

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

  // TODOO: These should be  refs
  size_t
  read_ins(instruction& ins) {
    if (isReader_) {
      return -1;
    }
    return fread(&ins, sizeof(ins), 1, ins_file_);
  }

  size_t
  read_bbl(basicblock& bbl) {
    if (isReader_) {
      return -1;
    }
    return fread(&bbl, sizeof(bbl), 1, bbl_file_);
  }

  size_t
  read_rtn(routine& rtn) {
    if (isReader_) {
      return -1;
    }
    return fread(&rtn, sizeof(rtn), 1, rtn_file_);
  }

  size_t
  write_ins(instruction& ins) {
    if (isReader_) {
      return -1;
    }
    return fwrite(&ins, sizeof(ins), 1, ins_file_);
  }

  size_t
  write_bbl(basicblock& bbl) {
    if (isReader_) {
      return -1;
    }
    return fwrite(&bbl, sizeof(bbl), 1, bbl_file_);
  }

  size_t
  write_rtn(routine& rtn) {
    if (isReader_) {
      return -1;
    }
    return fwrite(&rtn, sizeof(rtn), 1, rtn_file_);
  }
};
} // namespace libphase
