
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs
 *  and could serve as the starting point for developing your first PIN tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "pin.H"
#include "utils.h"
#include "xed/xed-category-enum.h"

#define NUM_INSTR_DESTINATIONS 2
#define NUM_INSTR_SOURCES 4

using namespace libphase;

// RESOURCES
// https://software.intel.com/sites/landingpage/pintool/docs/81205/Pin/html/group__INS__BASIC__API__GEN__IA32.html
// https://software.intel.com/sites/landingpage/pintool/docs/81205/Pin/html/group__RTN__BASIC__API.html
// https://software.intel.com/sites/landingpage/pintool/docs/81205/Pin/html/group__BBL__BASIC__API.html
// https://software.intel.com/sites/landingpage/pintool/docs/97503/Pin/html/group__INST__ARGS.html

// To collect
// - category
// - opcode
// - branching
//   - is branch
//   - is call
//   - taken / not taken
//   - direct / indirect
//   - conditional / unconditional
//   - forward / backwards
// - routines

/* ================================================================== */
// Global variables
/* ================================================================== */

UINT64 instrCount = 0;

std::string program_name;
std::string output_dir;
std::ofstream progress_file;
FILE* ins_file;
FILE* bbl_file;
FILE* rtn_file;
std::ofstream meta_file;

bool ins_file_closed = false;
bool tracing_on = false;

instruction curr_instr;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobINSPath(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "i",
    "libphase.ins.gz",
    "specify file name for libphase instruction stream");

KNOB<string> KnobBBLPath(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "b",
    "libphase.bbl.gz",
    "specify file name for libphase basic block stream");

KNOB<string> KnobRTNPath(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "r",
    "libphase.rtn.gz",
    "specify file name for libphase routine stream");

KNOB<string> KnobOutputDirectory(
    KNOB_MODE_WRITEONCE, "pintool", "d", "tmp", "specify output directory");

KNOB<string> KnobProgressPath(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "p",
    "libphase.progress",
    "specify file name to output progress");

KNOB<string> KnobMetaPath(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "m",
    "libphase.meta",
    "specify file name to output meta data");

KNOB<UINT64> KnobSkipInstructions(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "s",
    "0",
    "How many instructions to skip before tracing begins");

KNOB<UINT64> KnobTraceInstructions(
    KNOB_MODE_WRITEONCE,
    "pintool",
    "i",
    "1000000",
    "How many instructions to trace");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32
Usage() {
  std::cerr
      << "This tool creates a register and memory access trace" << std::endl
      << "Specify the output trace file with -o" << std::endl
      << "Specify the number of instructions to skip before tracing with -s"
      << std::endl
      << "Specify the number of instructions to trace with -t" << std::endl
      << std::endl;

  std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;

  return -1;
}

void
finalize() {
  std::cout << "Traced " << instrCount << " instructions" << std::endl;
  progress_file << "Instruction count: " << instrCount << std::endl;
  if (!ins_file_closed) {
    fclose(ins_file);
    fclose(bbl_file);
    fclose(rtn_file);
    ins_file_closed = true;
  }
  meta_file.close();
  progress_file.close();
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

void
BeginInstruction(VOID* ip, UINT32 routine_id, UINT32 opcode, UINT32 category) {
  // cerr << "1 Begin: " << (unsigned long long int)ip << endl;
  if (instrCount % 1000000 == 0) {
    progress_file << program_name << ": " << instrCount << " instructions"
                  << std::endl;
  }
  instrCount++;
  if (instrCount > KnobSkipInstructions.Value()) {
    tracing_on = true;

    if (instrCount >
        (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
      tracing_on = false;
    }
  }

  if (!tracing_on)
    return;

  // reset the current instruction
  curr_instr.ip = (unsigned long long int)ip;
  curr_instr.routine_id = routine_id;

  curr_instr.branch_info = 0;
  curr_instr.opcode = opcode;
  curr_instr.category = category;

  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    curr_instr.destination_registers[i] = 0;
    curr_instr.destination_memory[i] = 0;
  }

  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    curr_instr.source_registers[i] = 0;
    curr_instr.source_memory[i] = 0;
  }
}

void
EndInstruction() {
  if (instrCount > KnobSkipInstructions.Value()) {
    tracing_on = true;

    if (instrCount <=
        (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
      fwrite(&curr_instr, sizeof(curr_instr), 1, ins_file);
    } else {
      finalize();
      exit(0);
    }
  }
}

void
BranchHandler(
    BOOL is_taken,
    BOOL is_branch,
    BOOL is_call,
    BOOL is_direct,
    BOOL is_cond,
    BOOL is_fwd,
    BOOL is_ret) {
  if (is_taken) {
    curr_instr.branch_info |= BRANCH::taken;
  }

  if (is_branch) {
    curr_instr.branch_info |= BRANCH::branch;
  }

  if (is_call) {
    curr_instr.branch_info |= BRANCH::call;
  }

  if (is_direct) {
    curr_instr.branch_info |= BRANCH::direct;
  }

  if (is_cond) {
    curr_instr.branch_info |= BRANCH::cond;
  }

  if (is_fwd) {
    curr_instr.branch_info |= BRANCH::fwd;
  }

  if (is_ret) {
    curr_instr.branch_info |= BRANCH::ret;
  }
}

void
RegRead(UINT32 i, UINT32 index) {
  if (!tracing_on)
    return;

  REG r = (REG)i;

  // check to see if this register is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (curr_instr.source_registers[i] == ((unsigned char)r)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (curr_instr.source_registers[i] == 0) {
        curr_instr.source_registers[i] = (unsigned char)r;
        break;
      }
    }
  }
}

void
RegWrite(REG i, UINT32 index) {
  if (!tracing_on)
    return;

  REG r = (REG)i;

  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (curr_instr.destination_registers[i] == ((unsigned char)r)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (curr_instr.destination_registers[i] == 0) {
        curr_instr.destination_registers[i] = (unsigned char)r;
        break;
      }
    }
  }
  /*
     if(index==0)
     {
     curr_instr.destination_register = (unsigned long long int)r;
     }
     */
}

void
MemoryRead(VOID* addr, UINT32 index, UINT32 read_size) {
  if (!tracing_on)
    return;

  // check to see if this memory read location is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
    if (curr_instr.source_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (curr_instr.source_memory[i] == 0) {
        curr_instr.source_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
}

void
MemoryWrite(VOID* addr, UINT32 index) {
  if (!tracing_on)
    return;

  // check to see if this memory write location is already in the list
  int already_found = 0;
  for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
    if (curr_instr.destination_memory[i] == ((unsigned long long int)addr)) {
      already_found = 1;
      break;
    }
  }
  if (already_found == 0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (curr_instr.destination_memory[i] == 0) {
        curr_instr.destination_memory[i] = (unsigned long long int)addr;
        break;
      }
    }
  }
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

// Is called for every instruction and instruments reads and writes
VOID
Instruction(INS ins, VOID* v) {
  // begin each instruction with this function
  UINT32 opcode = INS_Opcode(ins);
  std::string opcode_string = OPCODE_StringShort(opcode);
  UINT32 category = INS_Category(ins);
  std::string category_string = CATEGORY_StringShort(category);

  INS_InsertCall(
      ins,
      IPOINT_BEFORE,
      (AFUNPTR)BeginInstruction,
      IARG_INST_PTR,
      IARG_UINT32,
      RTN_Id(INS_Rtn(ins)),
      IARG_UINT32,
      opcode,
      IARG_UINT32,
      category,
      IARG_END);

  // instrument branch instructions
  if (INS_IsBranchOrCall(ins)) {
    INS_InsertCall(
        ins,
        IPOINT_BEFORE,
        (AFUNPTR)BranchHandler,
        IARG_BRANCH_TAKEN,
        IARG_BOOL,
        INS_IsBranch(ins),
        IARG_BOOL,
        INS_IsCall(ins),
        IARG_BOOL,
        INS_IsDirectBranchOrCall(ins),
        IARG_BOOL,
        category == XED_CATEGORY_COND_BR,
        IARG_BOOL,
        INS_Address(INS_Next(ins)) > INS_Address(ins),
        IARG_BOOL,
        INS_IsRet(ins),
        IARG_END);
  }

  // instrument register reads
  UINT32 readRegCount = INS_MaxNumRRegs(ins);
  for (UINT32 i = 0; i < readRegCount; i++) {
    UINT32 regNum = INS_RegR(ins, i);

    INS_InsertCall(
        ins,
        IPOINT_BEFORE,
        (AFUNPTR)RegRead,
        IARG_UINT32,
        regNum,
        IARG_UINT32,
        i,
        IARG_END);
  }

  // instrument register writes
  UINT32 writeRegCount = INS_MaxNumWRegs(ins);
  for (UINT32 i = 0; i < writeRegCount; i++) {
    UINT32 regNum = INS_RegW(ins, i);

    INS_InsertCall(
        ins,
        IPOINT_BEFORE,
        (AFUNPTR)RegWrite,
        IARG_UINT32,
        regNum,
        IARG_UINT32,
        i,
        IARG_END);
  }

  // instrument memory reads and writes
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      UINT32 read_size = INS_MemoryReadSize(ins);

      INS_InsertCall(
          ins,
          IPOINT_BEFORE,
          (AFUNPTR)MemoryRead,
          IARG_MEMORYOP_EA,
          memOp,
          IARG_UINT32,
          memOp,
          IARG_UINT32,
          read_size,
          IARG_END);
    }
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertCall(
          ins,
          IPOINT_BEFORE,
          (AFUNPTR)MemoryWrite,
          IARG_MEMORYOP_EA,
          memOp,
          IARG_UINT32,
          memOp,
          IARG_END);
    }
  }

  // finalize each instruction with this function
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)EndInstruction, IARG_END);
}

VOID
WriteBasicBlock(ADDRINT address, UINT32 num_ins) {
  if (instrCount > KnobSkipInstructions.Value() &&
      instrCount <=
          (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
    basicblock bbl;
    bbl.address = address;
    bbl.num_ins = num_ins;
    fwrite(&bbl, sizeof(bbl), 1, bbl_file);
  }
}

// Is called for every trace and instruments reads and writes
VOID
Trace(TRACE trace, VOID* v) {
  // Visit every basic block  in the trace
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    // Insert a call to docount before every bbl, passing the number of
    // instructions
    BBL_InsertCall(
        bbl,
        IPOINT_BEFORE,
        (AFUNPTR)WriteBasicBlock,
        IARG_ADDRINT,
        BBL_Address(bbl),
        IARG_UINT32,
        BBL_NumIns(bbl),
        IARG_END);
  }
}

VOID WriteRoutine(UINT32 id, UINT32 num_ins) {
  if (instrCount > KnobSkipInstructions.Value() &&
      instrCount <=
          (KnobTraceInstructions.Value() + KnobSkipInstructions.Value())) {
    routine rtn;
    rtn.id = id;
    rtn.num_ins = num_ins;
    fwrite(&rtn, sizeof(rtn), 1, rtn_file);
  }
}

// Is called for every routine and instruments reads and writes
VOID
Routine(RTN rtn, VOID* v) {
  // std::cout << "Function: " << RTN_Name(rtn) << std::endl;
  RTN_InsertCall(rtn,
      IPOINT_BEFORE,
      (AFUNPTR)WriteRoutine,
      IARG_UINT32,
      RTN_Id(rtn),
      IARG_UINT32,
      RTN_NumIns(rtn));
}

// Is called for every image and instruments reads and writes
VOID
Image(IMG img, VOID* v) {
  if (IMG_IsMainExecutable(img)) {
    meta_file << "Main: " << IMG_Name(img) << std::endl;
  } else {
    meta_file << "Image: " << IMG_Name(img) << std::endl;
  }
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID
Fini(INT32 code, VOID* v) {
  finalize();
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet
 * started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
int
main(int argc, char* argv[]) {
  // Initialize PIN library. Print help message if -h(elp) is specified
  // in the command line or the command line is invalid
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  std::stringstream ss;
  // ss << "mkdir -p " << KnobOutputDirectory.Value().c_str();
  // std::system(ss.str().c_str());

  program_name = std::string(argv[0]);

  ss.str(std::string());
  ss << KnobOutputDirectory.Value().c_str() << "/"
     << KnobProgressPath.Value().c_str();
  progress_file.open(ss.str().c_str());

  ss.str(std::string());
  ss << "gzip -9c > " << KnobOutputDirectory.Value().c_str() << "/"
     << KnobINSPath.Value().c_str();
  ins_file = popen(ss.str().c_str(), "w");

  ss.str(std::string());
  ss << "gzip -9c > " << KnobOutputDirectory.Value().c_str() << "/"
     << KnobBBLPath.Value().c_str();
  bbl_file = popen(ss.str().c_str(), "w");

  ss.str(std::string());
  ss << "gzip -9c > " << KnobOutputDirectory.Value().c_str() << "/"
     << KnobRTNPath.Value().c_str();
  rtn_file = popen(ss.str().c_str(), "w");

  ss.str(std::string());
  ss << KnobOutputDirectory.Value().c_str() << "/"
     << KnobMetaPath.Value().c_str();
  meta_file.open(ss.str().c_str());

  if (!ins_file) {
    std::cout << "Couldn't open ins file. Exiting." << std::endl;
    exit(1);
  }
  if (!bbl_file) {
    std::cout << "Couldn't open bbl file. Exiting." << std::endl;
    exit(1);
  }
  if (!rtn_file) {
    std::cout << "Couldn't open rtn file. Exiting." << std::endl;
    exit(1);
  }

  PIN_InitSymbols();

  // Register function to be called whenever there is a new image
  IMG_AddInstrumentFunction(Image, 0);

  // Register function to be called to instrument instructions
  INS_AddInstrumentFunction(Instruction, 0);

  // Register function to be called to instrument bassic blocks
  TRACE_AddInstrumentFunction(Trace, 0);

  // Register function to be called to instrument routines
  RTN_AddInstrumentFunction(Routine, 0);

  // Register function to be called when the application exits
  PIN_AddFiniFunction(Fini, 0);

  std::cerr << "===============================================" << std::endl;
  std::cerr
      << "This application is instrumented by the libphase Trace Generator"
      << std::endl;
  std::cerr << "Trace saved in " << KnobINSPath.Value() << std::endl;
  std::cerr << "===============================================" << std::endl;

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
