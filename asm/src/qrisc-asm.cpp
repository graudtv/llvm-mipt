#include "Assembler.h"
#include <iostream>
#include <fstream>
#include <llvm/Support/CommandLine.h>

using namespace qrisc;
using namespace llvm;

cl::OptionCategory Options("Assembler options");
cl::opt<bool> PrintAsm("print", cl::desc("Print readable form of processed assembler"), cl::cat(Options));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"), cl::init("-"), cl::cat(Options));
cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"), cl::cat(Options));

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv, "qrisc assembly tool");

  Assembler &Asm = Assembler::getInstance();
  Asm.parseFileOrStdin(InputFilename);

  std::ostream *Os = &std::cout;
  std::ofstream Ofs;
  if (!OutputFilename.empty()) {
    Ofs.open(OutputFilename);
    if (!Ofs.is_open()) {
      std::cerr << "Failed to open file '" << OutputFilename << "' for writing\n";
      exit(1);
    }
    Os = &Ofs;
  }

  if (PrintAsm)
    Asm.print(*Os);
  else
    Asm.write(*Os);
  return 0;
}
