#include "Codegen.h"
#include "Parser.h"
#include "Sema.h"
#include <filesystem>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/CommandLine.h>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Host.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>

using namespace mercy;
namespace cl = llvm::cl;

// clang-format off
cl::OptionCategory Options("Frontend options");
cl::opt<bool> OptC("c",
    cl::desc("Compile or assemble the source files, but do not link"),
    cl::cat(Options));
cl::opt<bool> OptEmitLLVM("emit-llvm",
    cl::desc("Emit llvm IR"),
    cl::cat(Options));
cl::opt<bool> OptDumpAST("dump-ast",
    cl::desc("Dump AST after parser, before semantic analysis"),
    cl::cat(Options));
cl::opt<std::string> OptOutputFilename("o",
    cl::desc("Output filename"),
    cl::cat(Options));

cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::cat(Options));
// clang-format on

namespace {

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

enum class FrontendAction {
  EmitObject, // -c
  EmitLL,     // --emit-llvm
  DumpAST     // --dump-ast
};

FrontendAction getFrontendAction() {
  if (OptDumpAST)
    return FrontendAction::DumpAST;
  if (OptEmitLLVM)
    return FrontendAction::EmitLL;
  if (OptC)
    return FrontendAction::EmitObject;
  llvm::errs() << "mercy: no action specified\n";
  exit(0);
}

std::string getOutputFilename(FrontendAction Action) {
  if (!OptOutputFilename.empty())
    return OptOutputFilename;
  if (InputFilename.empty() || InputFilename == "-") {
    switch (Action) {
    case FrontendAction::DumpAST:
      return "";
    case FrontendAction::EmitLL:
      return "";
    case FrontendAction::EmitObject:
      return "output.o";
    }
  }
  std::filesystem::path Path = std::string{InputFilename};
  switch (Action) {
  case FrontendAction::DumpAST:
    return "";
  case FrontendAction::EmitLL:
    return Path.replace_extension(".ll");
  case FrontendAction::EmitObject:
    return Path.replace_extension(".o");
  }
  llvm_unreachable("invalid Action");
}

} // namespace

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv,
                              "Mercy Programming Language compiler");
  auto Action = getFrontendAction();
  auto OutputFilename = getOutputFilename(Action);

  auto AST = parseFileOrStdin(InputFilename);
  if (Action == FrontendAction::DumpAST) {
    AST->print(llvm::errs());
    exit(0);
  }

  Sema S;
  S.run(AST.get());

  Codegen Gen;
  Gen.run(AST.get(), S);

  auto M = Gen.takeResult();

  std::unique_ptr<llvm::raw_fd_ostream> OutputFile;
  if (!OutputFilename.empty() && OutputFilename != "-")
    OutputFile = openFile(OutputFilename);
  llvm::raw_fd_ostream &OS = OutputFile ? *OutputFile : llvm::outs();

  if (Action == FrontendAction::EmitLL) {
    OS << *M;
    verifyModule(*M, &llvm::errs());
    exit(0);
  }

  assert(Action == FrontendAction::EmitObject);

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  M->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    llvm::errs() << Error;
    exit(1);
  }

  auto CPU = "generic";
  auto Features = "";
  auto TM = Target->createTargetMachine(TargetTriple, CPU, Features, {},
                                        llvm::Reloc::PIC_);

  M->setDataLayout(TM->createDataLayout());

  llvm::legacy::PassManager CGPass;
  auto FileType = llvm::CodeGenFileType::CGFT_ObjectFile;

  if (TM->addPassesToEmitFile(CGPass, OS, nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return 1;
  }
  CGPass.run(*M);
  return 0;
}
