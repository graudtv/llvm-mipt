#include "Simulator.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/TargetSelect.h>

using namespace qrisc;
using namespace llvm;

cl::opt<bool> PrintIR("print-ir", cl::desc("Print generated IR"));
cl::opt<bool> RunProgram("run", cl::desc("Run program on simulator"));
cl::opt<bool> MemTrace("memtrace", cl::desc("Print all load-store operations"));

namespace {

uint32_t SimRegFile[REG_COUNT];
std::vector<std::byte> SimMemory;

void handleStore(uint32_t Value, uint32_t Addr) {
  if (MemTrace)
    outs() << formatv("write {0:x} -> [ {1:x} ]\n", Value, Addr);
  if (!(Addr & MMIO_MASK)) {
    if (Addr >= SimMemory.size()) {
      errs() << formatv("TRAP: write to illegal address {0:x}\n", Addr);
      exit(1);
    }
    memcpy(SimMemory.data() + Addr, &Value, sizeof(Value));
    return;
  }
  if (Addr == MMIO_IO_STDOUT) {
    outs() << static_cast<char>(Value);
    return;
  }
  errs() << formatv("TRAP: write to illegal MMIO address {0:x}\n", Addr);
  exit(1);
}

uint32_t handleLoad(uint32_t Addr) {
  if (MemTrace)
    outs() << formatv("read [{0:x}]\n", Addr);
  if (!(Addr & MMIO_MASK)) {
    if (Addr >= SimMemory.size()) {
      errs() << formatv("TRAP: read from illegal address {0:x}\n", Addr);
      exit(1);
    }
    uint32_t Value;
    memcpy(&Value, SimMemory.data() + Addr, sizeof(Value));
    return Value;
  }
  if (Addr == MMIO_IO_STDIN) {
    return getchar();
  }
  errs() << formatv("TRAP: read from illegal MMIO address {0:x}\n", Addr);
  exit(1);
}

} // namespace

void qrisc::simulate(const std::vector<Instr> &Instrs,
                     const SimulationOptions &SimOpts) {
  LLVMContext Ctx;
  auto M = std::make_unique<Module>("top", Ctx);
  IRBuilder<> Builder(Ctx);

  ArrayType *RegFileTy = ArrayType::get(Builder.getInt32Ty(), REG_COUNT);
  Value *RegFile = M->getOrInsertGlobal("__reg_file", RegFileTy);

  auto getRegPtr = [&Builder, RegFile, RegFileTy](reg_t Idx) -> Value * {
    return Builder.CreateConstInBoundsGEP2_32(RegFileTy, RegFile, 0, Idx);
  };
  auto getReg = [&](reg_t Idx) -> Value * {
    return Builder.CreateLoad(Builder.getInt32Ty(), getRegPtr(Idx));
  };

  // declare void @__builtin_store(i32 %value, i32 %addr)
  FunctionCallee LazyStore =
      M->getOrInsertFunction("__store", Builder.getVoidTy(),
                             Builder.getInt32Ty(), Builder.getInt32Ty());

  // declare i32 @__builtin_load(i32 %addr)
  FunctionCallee LazyLoad = M->getOrInsertFunction(
      "__load", Builder.getInt32Ty(), Builder.getInt32Ty());

  // declare void @main()
  FunctionType *MainFuncTy = FunctionType::get(Builder.getVoidTy(), false);
  Function *MainFunc =
      Function::Create(MainFuncTy, Function::ExternalLinkage, "main", M.get());
  BasicBlock *Entry = BasicBlock::Create(Ctx, "entry", MainFunc);
  Builder.SetInsertPoint(Entry);

  for (auto Ins : Instrs) {
    auto Opcode = Ins.getOpcode();
    if (Opcode == OPCODE_ADD) {
      Value *V = Builder.CreateAdd(getReg(Ins.r2()), getReg(Ins.r3()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_AND) {
      Value *V = Builder.CreateAnd(getReg(Ins.r2()), getReg(Ins.r3()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_OR) {
      Value *V = Builder.CreateOr(getReg(Ins.r2()), getReg(Ins.r3()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_XOR) {
      Value *V = Builder.CreateXor(getReg(Ins.r2()), getReg(Ins.r3()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_REMU) {
      Value *V = Builder.CreateURem(getReg(Ins.r2()), getReg(Ins.r3()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_ADDI) {
      Value *V =
          Builder.CreateAdd(getReg(Ins.r2()), Builder.getInt32(Ins.imm()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_ORI) {
      Value *V =
          Builder.CreateOr(getReg(Ins.r2()), Builder.getInt32(Ins.imm()));
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_LUI) {
      Value *Shifted =
          Builder.CreateShl(Builder.getInt32(Ins.imm()), Builder.getInt32(16));
      Value *V = Builder.CreateOr(getReg(Ins.r2()), Shifted);
      Builder.CreateStore(V, getRegPtr(Ins.r1()));
    } else if (Opcode == OPCODE_STORE) {
      Value *Addr =
          Builder.CreateAdd(getReg(Ins.r2()), Builder.getInt32(Ins.imm()));
      Value *V = Builder.CreateCall(LazyStore, {getReg(Ins.r1()), Addr});
    } else {
      errs() << "Unhandled instruction " << Ins.getMnemonic() << "\n";
    }
  }
  Builder.CreateRetVoid();

  if (PrintIR)
    outs() << *M;

  verifyModule(*M);

  if (!RunProgram)
    return;

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  ExecutionEngine *EE =
      EngineBuilder(std::unique_ptr<Module>(std::move(M))).create();
  EE->InstallLazyFunctionCreator([&](const std::string &FuncName) -> void * {
    return StringSwitch<void *>(FuncName)
        .Case("__store", reinterpret_cast<void *>(handleStore))
        .Default(nullptr);
  });
  EE->addGlobalMapping(cast<GlobalValue>(RegFile),
                       static_cast<void *>(SimRegFile));
  EE->finalizeObject();
  if (EE->hasError()) {
    errs() << "\nEngine error: " << EE->getErrorMessage() << "\n";
    exit(1);
  }
  outs() << "\nRunning code...\n";
  EE->runFunctionAsMain(MainFunc, {}, nullptr);
}
