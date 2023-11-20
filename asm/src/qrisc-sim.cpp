#include "Assembler.h"
#include "Simulator.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>
#include <sim.h>

using namespace qrisc;
using namespace llvm;

cl::OptionCategory Options("Simulator options");
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::Required, cl::cat(Options));
cl::opt<unsigned> MemorySize("memory-size", cl::desc("RAM size in bytes"),
                             cl::init(32 * 1024));
cl::opt<unsigned> StackAddr("stack-addr", cl::desc("Stack address"),
                            cl::init(16 * 1024));
cl::opt<bool> Trace("trace", cl::desc("Print all executed instructions"));
cl::opt<bool> TraceMem("trace-mem",
                       cl::desc("Print memory read-write operations"));
cl::opt<bool> TraceReg("trace-reg",
                       cl::desc("Print write operations to registers"));

class Simulator {
  reg_t RegFile[REG_COUNT] = {};
  std::vector<std::byte> Memory;
  reg_t SimPixelColor;
  reg_t SimPixelShape;

  reg_t getPC() const { return RegFile[REG_PC]; }
  void handleLoad(const Instr &Ins);
  void handleStore(const Instr &Ins);

  uint32_t readUintFromMemory(uint32_t Addr) {
    return *reinterpret_cast<uint32_t *>(Memory.data() + Addr);
  }

  void writeReg(unsigned Idx, reg_t Value) {
    assert(Idx < REG_COUNT && "invalid index");
    if (Idx == REG_PC) {
      errs() << "TRAP: write to rpc\n";
      exit(1);
    }
    if (Idx == REG_ZERO) // writes to zero reg are ignored
      return;
    RegFile[Idx] = Value;
    if (TraceReg)
      errs() << "REG: r" << Idx << " <- " << Value << "\n";
  }

public:
  void run(const std::vector<Instr> &Instrs);
};

void Simulator::handleLoad(const Instr &Ins) {
  uint32_t Addr = RegFile[Ins.r2()] + Ins.imm();
  uint32_t Res;
  if ((Addr & MMIO_MASK) != MMIO_MASK) {
    if (Addr >= Memory.size()) {
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
  writeReg(Ins.r1(), Res);
  if (TraceMem)
    errs() << formatv("MEM: read {0:x} <- [{1:x}]\n", Res, Addr);
}

void Simulator::handleStore(const Instr &Ins) {
  uint32_t Addr = RegFile[Ins.r2()] + Ins.imm();
  uint32_t Value = RegFile[Ins.r1()];
  if (TraceMem)
    errs() << formatv("MEM: write {0:x} -> [ {1:x} ]\n", Value, Addr);
  if ((Addr & MMIO_MASK) != MMIO_MASK) {
    if (Addr >= Memory.size()) {
      errs() << formatv("TRAP: write to illegal address {0:x}\n", Addr);
      exit(1);
    }
    memcpy(Memory.data() + Addr, &Value, sizeof(Value));
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

void Simulator::run(const std::vector<Instr> &Instrs) {
  size_t BinarySize = Instrs.size() * sizeof(Instr);
  if (MemorySize < BinarySize) {
    errs() << "Memory too small for this binary\n";
    exit(1);
  }
  if (StackAddr < BinarySize)
    errs() << "Warning: stack overlaps binary\n";

  SimPixelColor = 0xffffffff;
  SimPixelShape = 32;
  memset(RegFile, 0, sizeof(RegFile));
  Memory.resize(MemorySize);
  RegFile[REG_BP] = RegFile[REG_SP] = StackAddr;    // initialize rsp and rbp
  memcpy(Memory.data(), Instrs.data(), BinarySize); // load binary to memory

  while (RegFile[REG_PC] != BinarySize) {
    reg_t PC = getPC();
    reg_t NextPC = PC + 4;
    Instr Ins = Instr(readUintFromMemory(PC));
    opcode_t Opcode = Ins.getOpcode();

    if (Trace)
      Ins.print(std::cerr);

    switch (Opcode) {
    case OPCODE_ADD:
      writeReg(Ins.r1(), RegFile[Ins.r2()] + RegFile[Ins.r3()]);
      break;
    case OPCODE_ADDI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] + Ins.imm());
      break;
    case OPCODE_AND:
      writeReg(Ins.r1(), RegFile[Ins.r2()] & RegFile[Ins.r3()]);
      break;
    case OPCODE_ANDI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] & Ins.imm());
      break;
    case OPCODE_OR:
      writeReg(Ins.r1(), RegFile[Ins.r2()] | RegFile[Ins.r3()]);
      break;
    case OPCODE_ORI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] | Ins.imm());
      break;
    case OPCODE_XOR:
      writeReg(Ins.r1(), RegFile[Ins.r2()] ^ RegFile[Ins.r3()]);
      break;
    case OPCODE_XORI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] ^ Ins.imm());
      break;
    case OPCODE_SUB:
      writeReg(Ins.r1(), RegFile[Ins.r2()] - RegFile[Ins.r3()]);
      break;
    case OPCODE_SUBI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] - Ins.imm());
      break;
    case OPCODE_MUL:
      writeReg(Ins.r1(), RegFile[Ins.r2()] * RegFile[Ins.r3()]);
      break;
    case OPCODE_MULI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] * Ins.imm());
      break;
    case OPCODE_DIVU:
      writeReg(Ins.r1(), RegFile[Ins.r2()] / RegFile[Ins.r3()]);
      break;
    case OPCODE_DIVIU:
      writeReg(Ins.r1(), RegFile[Ins.r2()] / Ins.imm());
      break;
    case OPCODE_REMU:
      writeReg(Ins.r1(), RegFile[Ins.r2()] % RegFile[Ins.r3()]);
      break;
    case OPCODE_REMIU:
      writeReg(Ins.r1(), RegFile[Ins.r2()] % Ins.imm());
      break;
    case OPCODE_SLT:
      writeReg(Ins.r1(),
               ((int32_t)RegFile[Ins.r2()] < (int32_t)RegFile[Ins.r3()]) ? 1
                                                                         : 0);
      break;
    case OPCODE_SLTI:
      writeReg(Ins.r1(),
               ((int32_t)RegFile[Ins.r2()] < (int32_t)Ins.imm()) ? 1 : 0);
      break;
    case OPCODE_SLTU:
      writeReg(Ins.r1(), (RegFile[Ins.r2()] < RegFile[Ins.r3()]) ? 1 : 0);
      break;
    case OPCODE_SLTIU:
      writeReg(Ins.r1(), (RegFile[Ins.r2()] < Ins.imm()) ? 1 : 0);
      break;
    case OPCODE_LUI:
      writeReg(Ins.r1(), RegFile[Ins.r2()] | (Ins.imm() << 16));
      break;
    case OPCODE_LOAD:
      handleLoad(Ins);
      break;
    case OPCODE_STORE:
      handleStore(Ins);
      break;
    case OPCODE_BEQ:
      if (RegFile[Ins.r1()] == RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_BNE:
      if (RegFile[Ins.r1()] != RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_BGT:
      if ((int32_t)RegFile[Ins.r1()] > (int32_t)RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_BGE:
      if ((int32_t)RegFile[Ins.r1()] >= (int32_t)RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_BLT:
      if ((int32_t)RegFile[Ins.r1()] < (int32_t)RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_BLE:
      if ((int32_t)RegFile[Ins.r1()] <= (int32_t)RegFile[Ins.r2()])
        NextPC = PC + ((int32_t)Ins.imm()) * 4;
      break;
    case OPCODE_JALR:
      NextPC = RegFile[Ins.r2()] + ((int32_t)Ins.imm()) * 4;
      writeReg(Ins.r1(), PC);
      break;
    default:
      errs() << formatv("TRAP: illegal opcode {0:x}\n", Ins.getOpcode());
      exit(1);
    };
    if (NextPC >= Memory.size()) {
      errs() << formatv("TRAP: branch or jump at illegal address {0:x}\n",
                        NextPC);
      exit(1);
    }
    RegFile[REG_PC] = NextPC;
  }
}

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv);

  Assembler &Asm = Assembler::getInstance();
  Asm.parseFileOrStdin(InputFilename);

  Simulator Sim;
  Sim.run(Asm.getInstructions());
  return 0;
}
