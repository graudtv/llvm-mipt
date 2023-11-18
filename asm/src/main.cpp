#include "Assembler.h"
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#if 0
using namespace llvm;

std::unique_ptr<AsmHandler> Handler;

struct LazyEnvironment {
  static constexpr uint32_t MemorySize = 32 * 1024;
  uint32_t Reg[32];
  uint32_t Mem[MemorySize];
};

class LazySimulator : public AsmHandler {
  LazyEnvironment Env;
  LLVMContext Ctx;
  std::unique_ptr<Module> M;
  IRBuilder<> Builder;
  Function *MainFunc;

  void handle_r_instr(opcode_t opcode, reg_t r1, reg_t r2, reg_t r3) override {
  }

  void handle_i_instr(opcode_t opcode, reg_t r1, reg_t r2, imm_t imm) override {
  }

  void handle_wi_instr(opcode_t opcode, reg_t r1, imm_t imm) override {
  }

  void handle_label(const std::string &label) override {}

  void finalize() override {
    llvm::outs() << "Success!\n";
  }

  Value *getReg(reg_t Idx) {
    GlobalVariable *RegFile = M->getNamedGlobal("__reg_file");
    return Builder.CreateConstInBoundsGEP2_32(RegFile->getType(), RegFile, 0, Idx);
  }

public:
  LazySimulator() : Ctx(), Builder(Ctx) {
    M = std::make_unique<Module>("top", Ctx);

    ArrayType *RegFileTy = ArrayType::get(Builder.getInt32Ty(), LazyEnvironment::MemorySize);
    M->getOrInsertGlobal("__reg_file", RegFileTy);

    // declare void @main()
    FunctionType *MainFuncTy = FunctionType::get(Builder.getVoidTy(), false);
    MainFunc = Function::Create(MainFuncTy, Function::ExternalLinkage, "main", M.get());

  }
};
#endif

int yyparse();

int main() {
  qrisc::Assembler &Asm = qrisc::Assembler::getInstance();
  Asm.parse(stdin);
  Asm.print(std::cout);

  return 0;
}
