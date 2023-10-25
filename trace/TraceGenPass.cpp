#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {

void ltrim(std::string &S) {
  S.erase(S.begin(), std::find_if(S.begin(), S.end(), [](unsigned char C) {
            return !std::isspace(C);
          }));
}

std::string getLabel(const BasicBlock &BB) {
  std::string Str;
  raw_string_ostream Os(Str);
  BB.printAsOperand(Os, false);
  return Str;
}

std::string toString(const Instruction &I) {
  std::string Str;
  raw_string_ostream Os(Str);
  Os << I;
  ltrim(Str);
  return Str;
}

class TraceGenPass : public FunctionPass {
  static char ID;

public:
  TraceGenPass() : FunctionPass(ID) {}

  FunctionCallee getTraceFunc(llvm::Module &M) {
    LLVMContext &Ctx = M.getContext();
    Type *ParamTys[] = {Type::getInt8Ty(Ctx)->getPointerTo()};
    Type *RetTy = Type::getInt32Ty(Ctx);
    FunctionType *FT = FunctionType::get(RetTy, ParamTys, false);
    return M.getOrInsertFunction("puts", FT);
  }

  virtual bool runOnFunction(Function &F) {
    FunctionCallee TraceFunc = getTraceFunc(*F.getParent());
    IRBuilder<> Builder(F.getParent()->getContext());
    Type *VoidTy = Builder.getVoidTy();

    /* Explicitly set names to instructions */
    int Id = 0;
    for (auto &BB : F) {
      BB.setName(std::to_string(Id++));
      for (auto &I : BB)
        if (I.getType() != VoidTy)
          I.setName(std::to_string(Id++));
    }

    for (auto &BB : F)
      for (auto &I : BB) {
        if (isa<PHINode>(I))
          continue;

        Builder.SetInsertPoint(&I);

        /* Trace function exits */
        if (isa<ReturnInst>(I))
          Builder.CreateCall(
              TraceFunc, Builder.CreateGlobalStringPtr(
                             ("; end function '" + F.getName() + "'").str()));
        /* Trace each instruction */
        Value *Instr = Builder.CreateGlobalStringPtr(toString(I));
        Builder.CreateCall(TraceFunc, {Instr});
      }

    /* Trace BB entrances */
    for (auto &BB : F) {
      auto Label = getLabel(BB);
      auto It = find_if(BB, [](auto &I) { return !isa<PHINode>(I); });
      Builder.SetInsertPoint(&*It);
      Builder.CreateCall(TraceFunc, Builder.CreateGlobalStringPtr(
                                        "; function '" + F.getName().str() +
                                        "' BB '" + Label + "'"));
    }

    /* Trace function entrances */
    Builder.SetInsertPoint(&F.getEntryBlock().front());
    Builder.CreateCall(TraceFunc,
                       Builder.CreateGlobalStringPtr(
                           ("; begin function '" + F.getName() + "'").str()));

    verifyFunction(F, &errs());
    return true;
  }
};

char TraceGenPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
void registerPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
  PM.add(new TraceGenPass);
}

RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                                      registerPass);

} // namespace
