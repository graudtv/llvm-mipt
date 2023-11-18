#include "Assembler.h"
#include "Simulator.h"
#include <iostream>
#include <llvm/Support/CommandLine.h>

using namespace qrisc;
using namespace llvm;

cl::opt<bool> PrintAsm("print-asm", cl::desc("Print parsed assembler"));

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv);

  Assembler &Asm = Assembler::getInstance();
  Asm.parse(stdin);

  if (PrintAsm)
    Asm.print(std::cout);

  SimulationOptions SimOpts;
  simulate(Asm.getInstructions(), SimOpts);
  return 0;
}
