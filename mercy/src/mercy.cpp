#include "Parser.h"
#include "Codegen.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/Verifier.h>

using namespace mercy;
namespace cl = llvm::cl;

// clang-format off
cl::OptionCategory Options("Driver options");
cl::opt<bool> OptC("c",
    cl::desc("Compile or assemble the source files, but do not link"),
    cl::cat(Options));
cl::opt<bool> OptS("S",
    cl::desc("Stop after the stage of compilation, do not assemble"),
    cl::cat(Options));
cl::opt<bool> OptEmitLLVM("emit-llvm",
    cl::desc("Emit llvm IR"),
    cl::cat(Options));
cl::opt<bool> OptDumpAST("dump-ast",
    cl::desc("Dump AST"),
    cl::cat(Options));
cl::opt<std::string> OutputFilename("o",
    cl::desc("Output filename"),
    cl::cat(Options));

cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::init("-"), cl::cat(Options));
// clang-format off

std::unique_ptr<llvm::raw_fd_ostream> openFile(const std::string &Filename) {
  std::error_code EC;
  auto File = std::make_unique<llvm::raw_fd_ostream>(Filename, EC);
  if (EC) {
    llvm::errs() << "Failed to open file '" << Filename << "': " << EC.message()
                 << '\n';
    exit(1);
  }
  return File;
}

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv,
                              "Mercy Programming Language compiler");
  if (OptC && OptS) {
    llvm::errs() << "-c and -S cannot be specified simulataneously\n";
    exit(1);
  }

  auto AST = parseFileOrStdin(InputFilename);

  std::unique_ptr<llvm::raw_fd_ostream> OutputFile;
  if (!OutputFilename.empty() && OutputFilename != "-")
    OutputFile = openFile(OutputFilename);
  llvm::raw_ostream &OS = OutputFile ? *OutputFile : llvm::outs();

  if (OptDumpAST) {
    AST->print(OS);
    return 0;
  }

  Codegen Gen;
  Gen.run(std::move(AST));

  std::unique_ptr<llvm::Module> M = Gen.takeResult();
  if (OptS && OptEmitLLVM) {
    OS << *M;
  }
  verifyModule(*M, &llvm::errs());

  return 0;
}
