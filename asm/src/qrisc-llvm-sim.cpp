#include "Assembler.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/TargetSelect.h>
#include <sim.h>

using namespace qrisc;
using namespace llvm;

cl::OptionCategory Options("Simulator options");
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::Required, cl::cat(Options));
cl::opt<unsigned> MemorySize("memory-size", cl::desc("RAM size in bytes"),
                             cl::init(512 * 1024), cl::cat(Options));
cl::opt<unsigned> StackAddr("stack-addr", cl::desc("Stack address"),
                            cl::init(16 * 1024), cl::cat(Options));
cl::opt<bool> TraceMem("trace-mem",
                       cl::desc("Print memory read-write operations"),
                       cl::cat(Options));
cl::opt<bool> PrintIR("print-ir",
                      cl::desc("Print generated IR instead of running"),
                      cl::cat(Options));

namespace {

reg_t SimRegFile[REG_COUNT];
std::vector<std::byte> SimMemory;
reg_t SimPixelColor = 0;
reg_t SimPixelShape = 0;

void handleStore(uint32_t Value, uint32_t Addr) {
  if (TraceMem)
    errs() << formatv("MEM: write {0:x} -> [ {1:x} ]\n", Value, Addr);
  if ((Addr & MMIO_MASK) != MMIO_MASK) {
    if (Addr >= SimMemory.size()) {
      errs() << formatv("TRAP: write to illegal address {0:x}\n", Addr);
      exit(1);
    }
    memcpy(SimMemory.data() + Addr, &Value, sizeof(Value));
  } else if (Addr == MMIO_IO_STDOUT) {
    outs() << static_cast<char>(Value);
  } else if (Addr == MMIO_SIM_CLEAR) {
    sim_clear(Value);
  } else if (Addr == MMIO_SIM_DISPLAY) {
    sim_display();
  } else if (Addr == MMIO_SIM_PIXEL_COLOR) {
    SimPixelColor = Value;
  } else if (Addr == MMIO_SIM_PIXEL_SHAPE) {
    SimPixelShape = Value;
  } else if (Addr == MMIO_SIM_SET_PIXEL) {
    sim_set_pixel((Value >> 16) & 0xffff, Value & 0xffff, SimPixelColor,
                  static_cast<sim_shape_t>(SimPixelShape));
  } else {
    errs() << formatv("TRAP: write to illegal MMIO address {0:x}\n", Addr);
    exit(1);
  }
}

uint32_t readUintFromMemory(uint32_t Addr) {
  return *reinterpret_cast<uint32_t *>(SimMemory.data() + Addr);
}

uint32_t handleLoad(uint32_t Addr) {
  uint32_t Res;
  if ((Addr & MMIO_MASK) != MMIO_MASK) {
    if (Addr >= SimMemory.size()) {
      errs() << formatv("TRAP: read from illegal address {0:x}\n", Addr);
      exit(1);
    }
    Res = readUintFromMemory(Addr);
  } else if (Addr == MMIO_IO_STDIN) {
    Res = getchar();
  } else if (Addr == MMIO_SIM_PIXEL_COLOR) {
    Res = SimPixelColor;
  } else if (Addr == MMIO_SIM_PIXEL_SHAPE) {
    Res = SimPixelShape;
  } else if (Addr == MMIO_SIM_RAND) {
    Res = sim_rand();
  } else {
    errs() << formatv("TRAP: read from illegal MMIO address {0:x}\n", Addr);
    exit(1);
  }
  if (TraceMem)
    errs() << formatv("MEM: read {0:x} <- [{1:x}]\n", Res, Addr);
  return Res;
}

} // namespace

void simulate(const std::vector<Instr> &Instrs) {
  LLVMContext Ctx;
  auto M = std::make_unique<Module>("top", Ctx);
  IRBuilder<> Builder(Ctx);

  ArrayType *RegFileTy = ArrayType::get(Builder.getInt32Ty(), REG_COUNT);
  Value *RegFile = M->getOrInsertGlobal("__reg_file", RegFileTy);
  reg_t InstrPC = 0;

  auto getRegPtr = [&Builder, RegFile, RegFileTy](reg_t Idx) -> Value * {
    return Builder.CreateConstInBoundsGEP2_32(RegFileTy, RegFile, 0, Idx);
  };
  auto readReg = [&](unsigned Idx) -> Value * {
    assert(Idx < REG_COUNT && "invalid index");
    if (Idx == REG_ZERO)
      return Builder.getInt32(0);
    if (Idx == REG_PC)
      return Builder.getInt32(InstrPC);
    return Builder.CreateLoad(Builder.getInt32Ty(), getRegPtr(Idx));
  };
  auto writeReg = [&](unsigned Idx, Value *V) {
    assert(Idx < REG_COUNT && "invalid index");
    if (Idx == REG_ZERO) // writes to zero reg are ignored
      return;
    Builder.CreateStore(V, getRegPtr(Idx));
  };

  // declare void @__store(i32 %value, i32 %addr)
  FunctionCallee LazyStore =
      M->getOrInsertFunction("__store", Builder.getVoidTy(),
                             Builder.getInt32Ty(), Builder.getInt32Ty());

  // declare i32 @__load(i32 %addr)
  FunctionCallee LazyLoad = M->getOrInsertFunction(
      "__load", Builder.getInt32Ty(), Builder.getInt32Ty());

  // declare void @main()
  FunctionType *MainFuncTy = FunctionType::get(Builder.getVoidTy(), false);
  Function *MainFunc =
      Function::Create(MainFuncTy, Function::ExternalLinkage, "main", M.get());
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", MainFunc);
  Builder.SetInsertPoint(BBEntry);

  /* Possible branch/jump destination address -> associated BasicBlock */
  std::unordered_map<uint32_t, BasicBlock *> BBJumpDest;
  BBJumpDest[0] = BBEntry;
  /* Gather information on jump and branches */
  for (auto Ins : Instrs) {
    if (Ins.isJump() && (Ins.r1() != REG_ZERO || Ins.r2() != REG_PC)) {
      errs() << "jalr with non-zero link register or non PC-relative jump "
                "address is not supported";
      exit(1);
    }
    if (Ins.isBranch() || Ins.isJump()) {
      uint32_t Addr = InstrPC + static_cast<simm_t>(Ins.imm()) * 4;
      if (!BBJumpDest[Addr])
        BBJumpDest[Addr] = BasicBlock::Create(Ctx, "bb", MainFunc);
      if (!BBJumpDest[InstrPC + 4])
        BBJumpDest[InstrPC + 4] = BasicBlock::Create(Ctx, "bb_cont", MainFunc);
    }
    InstrPC += 4;
  }

  InstrPC = 0;
  for (auto Ins : Instrs) {
    auto Opcode = Ins.getOpcode();
    Value *BranchCond = nullptr;
    if (Opcode == OPCODE_ADD) {
      writeReg(Ins.r1(),
               Builder.CreateAdd(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_AND) {
      writeReg(Ins.r1(),
               Builder.CreateAnd(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_OR) {
      writeReg(Ins.r1(),
               Builder.CreateOr(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_XOR) {
      writeReg(Ins.r1(),
               Builder.CreateXor(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_SUB) {
      writeReg(Ins.r1(),
               Builder.CreateSub(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_MUL) {
      writeReg(Ins.r1(),
               Builder.CreateMul(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_DIVU) {
      writeReg(Ins.r1(),
               Builder.CreateUDiv(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_REMU) {
      writeReg(Ins.r1(),
               Builder.CreateURem(readReg(Ins.r2()), readReg(Ins.r3())));
    } else if (Opcode == OPCODE_SLT) {
      Value *Cmp = Builder.CreateICmpSLT(readReg(Ins.r1()), readReg(Ins.r2()));
      writeReg(Ins.r1(), Builder.CreateZExt(Cmp, Builder.getInt32Ty()));
    } else if (Opcode == OPCODE_SLTU) {
      Value *Cmp = Builder.CreateICmpULT(readReg(Ins.r1()), readReg(Ins.r2()));
      writeReg(Ins.r1(), Builder.CreateZExt(Cmp, Builder.getInt32Ty()));
    } else if (Opcode == OPCODE_ADDI) {
      writeReg(Ins.r1(), Builder.CreateAdd(readReg(Ins.r2()),
                                           Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_ANDI) {
      writeReg(Ins.r1(), Builder.CreateAnd(readReg(Ins.r2()),
                                           Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_ORI) {
      writeReg(Ins.r1(), Builder.CreateOr(readReg(Ins.r2()),
                                          Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_XORI) {
      writeReg(Ins.r1(), Builder.CreateXor(readReg(Ins.r2()),
                                           Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_SUBI) {
      writeReg(Ins.r1(), Builder.CreateSub(readReg(Ins.r2()),
                                           Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_MULI) {
      writeReg(Ins.r1(), Builder.CreateMul(readReg(Ins.r2()),
                                           Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_DIVIU) {
      writeReg(Ins.r1(), Builder.CreateUDiv(readReg(Ins.r2()),
                                            Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_REMIU) {
      writeReg(Ins.r1(), Builder.CreateURem(readReg(Ins.r2()),
                                            Builder.getInt32(Ins.imm())));
    } else if (Opcode == OPCODE_SLTI) {
      Value *Cmp =
          Builder.CreateICmpSLT(readReg(Ins.r1()), Builder.getInt32(Ins.imm()));
      writeReg(Ins.r1(), Builder.CreateZExt(Cmp, Builder.getInt32Ty()));
    } else if (Opcode == OPCODE_SLTIU) {
      Value *Cmp =
          Builder.CreateICmpULT(readReg(Ins.r1()), Builder.getInt32(Ins.imm()));
      writeReg(Ins.r1(), Builder.CreateZExt(Cmp, Builder.getInt32Ty()));
    } else if (Opcode == OPCODE_LUI) {
      Value *ShiftedImm =
          Builder.CreateShl(Builder.getInt32(Ins.imm()), Builder.getInt32(16));
      Value *Mask = Builder.getInt32(0xffff);
      Value *LowerBits = Builder.CreateAnd(readReg(Ins.r2()), Mask);
      writeReg(Ins.r1(), Builder.CreateOr(LowerBits, ShiftedImm));
    } else if (Opcode == OPCODE_LOAD) {
      Value *Addr =
          Builder.CreateAdd(readReg(Ins.r2()), Builder.getInt32(Ins.imm()));
      writeReg(Ins.r1(), Builder.CreateCall(LazyLoad, Addr));
    } else if (Opcode == OPCODE_STORE) {
      Value *Addr =
          Builder.CreateAdd(readReg(Ins.r2()), Builder.getInt32(Ins.imm()));
      Builder.CreateCall(LazyStore, {readReg(Ins.r1()), Addr});
    } else if (Opcode == OPCODE_BEQ) {
      BranchCond = Builder.CreateICmpEQ(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_BEQ) {
      BranchCond = Builder.CreateICmpNE(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_BGT) {
      BranchCond = Builder.CreateICmpSGT(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_BGE) {
      BranchCond = Builder.CreateICmpSGE(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_BLT) {
      BranchCond = Builder.CreateICmpSLT(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_BLE) {
      BranchCond = Builder.CreateICmpSLE(readReg(Ins.r1()), readReg(Ins.r2()));
    } else if (Opcode == OPCODE_JALR) {
      /* processed below */
    } else {
      assert((Ins.getType() == InstrType::Invalid) &&
             "valid instruction was not handled");
      /* Invalid instructions are silently ignored, because the may define
       * static data (e.g. via .word macros from assembly) */
    }

    /* Handle control flow */
    if (BranchCond) {
      assert(Ins.isBranch());
      uint32_t JumpAddr = InstrPC + static_cast<simm_t>(Ins.imm()) * 4;
      BasicBlock *TrueBranch = BBJumpDest[JumpAddr];
      BasicBlock *FalseBranch = BBJumpDest[InstrPC + 4];
      Builder.CreateCondBr(BranchCond, TrueBranch, FalseBranch);
      Builder.SetInsertPoint(FalseBranch);
    } else if (Ins.isJump()) {
      uint32_t JumpAddr = InstrPC + static_cast<simm_t>(Ins.imm()) * 4;
      Builder.CreateBr(BBJumpDest[JumpAddr]);
      Builder.SetInsertPoint(BBJumpDest[InstrPC + 4]);
    } else if (auto *BBNext = BBJumpDest[InstrPC + 4]) {
      Builder.CreateBr(BBNext);
      Builder.SetInsertPoint(BBNext);
    }
    InstrPC += 4;
  }
  Builder.CreateRetVoid();

  if (PrintIR) {
    outs() << *M;
    verifyModule(*M, &errs());
    return;
  }

  verifyModule(*M, &errs());

  /* Prepare memory and registers */
  size_t BinarySize = Instrs.size() * sizeof(Instr);
  SimPixelColor = 0xffffffff;
  SimPixelShape = 32;
  memset(SimRegFile, 0, sizeof(SimRegFile));
  SimMemory.clear();
  SimMemory.resize(MemorySize);
  SimRegFile[REG_BP] = SimRegFile[REG_SP] = StackAddr; // initialize rsp and rbp
  memcpy(SimMemory.data(), Instrs.data(), BinarySize); // load binary to memory

  /* Prepare and run ExecutionEngine */
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  ExecutionEngine *EE =
      EngineBuilder(std::unique_ptr<Module>(std::move(M))).create();
  EE->InstallLazyFunctionCreator([&](const std::string &FuncName) -> void * {
    return StringSwitch<void *>(FuncName)
        .Case("__load", reinterpret_cast<void *>(handleLoad))
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
  EE->runFunctionAsMain(MainFunc, {}, nullptr);
}

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv);

  Assembler &Asm = Assembler::getInstance();
  Asm.parseFileOrStdin(InputFilename);

  simulate(Asm.getInstructions());
  return 0;
}
