#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

namespace qrisc {

typedef unsigned opcode_t;
typedef unsigned reg_t;
typedef unsigned imm_t;

enum : unsigned {
  INSTR_OPCODE_SHIFT = 26,
  INSTR_R1_SHIFT = 21,
  INSTR_R2_SHIFT = 16,
  INSTR_R3_SHIFT = 11,

  REG_COUNT = 32,
  INSTR_R_MASK = REG_COUNT - 1,
  INSTR_IMM_MASK = 0xffff,
};

enum : uintptr_t {
  MMIO_IO_STDIN = 0xffffff00,
  MMIO_IO_STDOUT = 0xffffff01,

  MMIO_SIM_CLEAR = 0xffffff10,
  MMIO_SIM_DISPLAY = 0xffffff11,
  MMIO_SIM_PIXEL_COLOR = 0xffffff12,
  MMIO_SIM_PIXEL_SHAPE = 0xffffff13,
  MMIO_SIM_SET_PIXEL = 0xffffff14,
  MMIO_SIM_RAND = 0xffffff15,

  MMIO_MASK = 0xffffff00
};

enum : unsigned {
  OPCODE_ADD = 0x01,
  OPCODE_ADDI = 0x02,
  OPCODE_ADDM = 0x03,
  OPCODE_AND = 0x04,
  OPCODE_ANDI = 0x05,
  OPCODE_ANDM = 0x06,
  OPCODE_OR = 0x07,
  OPCODE_ORI = 0x08,
  OPCODE_ORM = 0x09,
  OPCODE_XOR = 0x0a,
  OPCODE_XORI = 0x0b,
  OPCODE_XORM = 0x0c,
  OPCODE_UREM = 0x0d,
  OPCODE_UREMI = 0x0e,
  OPCODE_UREMM = 0x0f,
  OPCODE_LOAD = 0x10,
  OPCODE_STORE = 0x11,
  OPCODE_LUI = 0x12,
  OPCODE_BEQ = 0x28,
  OPCODE_BNE = 0x29,
  OPCODE_BGT = 0x2a,
  OPCODE_BGE = 0x2b,
  OPCODE_BLT = 0x2c,
  OPCODE_BLE = 0x2d,
  OPCODE_JALR = 0x2e,
};

enum class InstrType { R, I };

inline InstrType getInstrType(opcode_t opcode) {
  switch (opcode) {
  case OPCODE_ADD:
  case OPCODE_AND:
  case OPCODE_OR:
  case OPCODE_XOR:
  case OPCODE_UREM:
    return InstrType::R;

  case OPCODE_ADDI:
  case OPCODE_ANDI:
  case OPCODE_ORI:
  case OPCODE_XORI:
  case OPCODE_UREMI:
  case OPCODE_ADDM:
  case OPCODE_ANDM:
  case OPCODE_ORM:
  case OPCODE_XORM:
  case OPCODE_UREMM:
  case OPCODE_LOAD:
  case OPCODE_STORE:
  case OPCODE_BEQ:
  case OPCODE_BNE:
  case OPCODE_BGT:
  case OPCODE_BGE:
  case OPCODE_BLT:
  case OPCODE_BLE:
  case OPCODE_JALR:
  case OPCODE_LUI:
    return InstrType::I;
  }
  assert(0 && "invalid opcode");
}

inline const char *getInstrMnemonic(opcode_t opcode) {
  switch (opcode) {
  // clang-format off
  case OPCODE_ADD: return "add";
  case OPCODE_ADDI: return "addi";
  case OPCODE_ADDM: return "addm";
  case OPCODE_AND: return "and";
  case OPCODE_ANDI: return "andi";
  case OPCODE_ANDM: return "andm";
  case OPCODE_OR: return "or";
  case OPCODE_ORI: return "ori";
  case OPCODE_ORM: return "orm";
  case OPCODE_XOR: return "xor";
  case OPCODE_XORI: return "xori";
  case OPCODE_XORM: return "xorm";
  case OPCODE_UREM: return "urem";
  case OPCODE_UREMI: return "uremi";
  case OPCODE_UREMM: return "uremm";
  case OPCODE_LOAD: return "load";
  case OPCODE_STORE: return "store";
  case OPCODE_LUI: return "lui";
  case OPCODE_BEQ: return "beq";
  case OPCODE_BNE: return "bne";
  case OPCODE_BGT: return "bgt";
  case OPCODE_BGE: return "bge";
  case OPCODE_BLT: return "blt";
  case OPCODE_BLE: return "ble";
  case OPCODE_JALR: return "jalr";
    // clang-format on
  }
  assert(0 && "invalid opcode");
}

class Instr {
  uint32_t Ins;
  Instr(uint32_t I) : Ins(I) {}

public:
  static Instr makeRInstr(opcode_t opcode, reg_t r1, reg_t r2, reg_t r3) {
    assert(!(r1 & ~INSTR_R_MASK) && "register out of bounds");
    assert(!(r2 & ~INSTR_R_MASK) && "register out of bounds");
    assert(!(r3 & ~INSTR_R_MASK) && "register out of bounds");
    assert(getInstrType(opcode) == InstrType::R && "must be R-instr");
    return Instr((opcode << INSTR_OPCODE_SHIFT) | (r1 << INSTR_R1_SHIFT) |
                 (r2 << INSTR_R2_SHIFT) | (r3 << INSTR_R3_SHIFT));
  }
  static Instr makeIInstr(opcode_t opcode, reg_t r1, reg_t r2, imm_t imm) {
    assert(!(r1 & ~INSTR_R_MASK) && "register out of bounds");
    assert(!(r2 & ~INSTR_R_MASK) && "register out of bounds");
    assert(!(imm & ~INSTR_IMM_MASK) && "immediate out of bounds");
    assert(getInstrType(opcode) == InstrType::I && "must be R-instr");
    return Instr((opcode << INSTR_OPCODE_SHIFT) | (r1 << INSTR_R1_SHIFT) |
                 (r2 << INSTR_R2_SHIFT) | imm);
  }

  opcode_t getOpcode() const { return Ins >> INSTR_OPCODE_SHIFT; }
  InstrType getType() const { return getInstrType(getOpcode()); }
  const char *getMnemonic() const { return getInstrMnemonic(getOpcode()); }

  reg_t r1() const { return (Ins >> INSTR_R1_SHIFT) & INSTR_R_MASK; }
  reg_t r2() const { return (Ins >> INSTR_R2_SHIFT) & INSTR_R_MASK; }
  reg_t r3() const {
    assert(getType() == InstrType::R && "Only R instructions have r3");
    return (Ins >> INSTR_R3_SHIFT) & INSTR_R_MASK;
  }
  reg_t imm() const {
    assert(getType() == InstrType::I && "Only I instructions have imm");
    return Ins & INSTR_IMM_MASK;
  }

  void print(std::ostream &Os = std::cout) const {
    auto Type = getType();
    Os << getMnemonic() << " r" << r1() << ", r" << r2();
    if (Type == InstrType::R)
      Os << ", r" << r3();
    else
      Os << ", " << imm();
    Os << "\n";
  }
};

} // namespace qrisc
