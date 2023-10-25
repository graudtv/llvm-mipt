#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <utility>
#include <vector>

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
    /* Insertion place -> string to dump */
    std::vector<std::pair<Instruction *, std::string>> TraceCalls;

    /* To preserve original names of BBs and values, first gather
     * information without modifying IR */
    for (auto &BB : F) {
      /* Trace BB entrances */
      auto It = find_if(BB, [](auto &I) { return !isa<PHINode>(I); });
      TraceCalls.emplace_back(&*It, "; function '" + F.getName().str() +
                                        "' BB '" + getLabel(BB) + "'");

      /* Trace each instruction */
      for (auto &I : BB) {
        auto It = std::find_if(I.getIterator(), BB.end(), [](auto &Instr) {
          return !isa<PHINode>(Instr);
        });
        TraceCalls.emplace_back(&*It, toString(I));
      }
    }

    /* Trace function entrances */
    Builder.SetInsertPoint(&F.getEntryBlock().front());
    Builder.CreateCall(TraceFunc,
                       Builder.CreateGlobalStringPtr(
                           ("; begin function '" + F.getName() + "'").str()));

    /* Insert prepared calls */
    for (auto &I : TraceCalls) {
      Builder.SetInsertPoint(I.first);
      Builder.CreateCall(TraceFunc, Builder.CreateGlobalStringPtr(I.second));
    }

    /* Trace function exits */
    for (auto &BB : F) {
      auto &I = BB.back();
      if (isa<ReturnInst>(I)) {
        Builder.SetInsertPoint(&I);
        Builder.CreateCall(TraceFunc,
                           Builder.CreateGlobalStringPtr(
                               ("; end function '" + F.getName() + "'").str()));
      }
    }

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
