#include <clipp.h>

#include "TraceIO.h"

using namespace std;
using namespace clipp;
using namespace libphase;

int
main(int argc, char* argv[]) {
  string ins_path;
  string bbl_path;
  string rtn_path;

  auto cli =
      (required("-i", "--ins-path").doc("Instruction stream path") &
           value("ins_path", ins_path),
       required("-b", "--bbl-path").doc("Basic block stream path") &
           value("bbl_path", bbl_path),
       required("-r", "--rtn-path").doc("Routine stream path") &
           value("rtn_path", rtn_path));

  if (!parse(argc, argv, cli)) {
    cout << make_man_page(cli, argv[0]);
    exit(1);
  }

  TraceIO io(true, ".", ins_path, bbl_path, rtn_path);

  while (io.read()) {

  }

  //while (io.read_bbl()) {
    //cout << io.curr_bbl.address << endl;
  //}
  //
  //while (io.read_ins()) {
    //cout << io.curr_ins.ip << endl;
  //}
  //while (io.read_rtn()) {
    //cout << io.curr_rtn.address << endl;
  //}
  return 0;
}
