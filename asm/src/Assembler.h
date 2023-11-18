#pragma once

#include "Instr.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace qrisc {

struct UnresolvedLabel {
  std::string Label;
  unsigned InstrIdx;
};

class Assembler {
  std::vector<Instr> Instrs;
  std::unordered_map<std::string, unsigned> Labels;
  std::vector<UnresolvedLabel> UnresolvedLabels;

  Assembler() = default;
  Assembler(const Assembler &) = delete;
  Assembler &operator =(const Assembler &) = delete;

  unsigned getNextInstrIdx() const { return Instrs.size(); }

public:
  static Assembler &getInstance();

  void parse(FILE *In);
  void writeToFile(const std::string &Filename);
  void write(std::ostream &Os);
  void print(std::ostream &Os);

  const std::vector<Instr> &getInstructions() const { return Instrs; }

  /* Invoked by parser */
  void appendLabel(const char *label);
  void appendRInstr(opcode_t opcode, reg_t r1, reg_t r2, reg_t r3);
  void appendIInstr(opcode_t opcode, reg_t r1, reg_t r2, imm_t imm);
  void appendIInstr(opcode_t opcode, reg_t r1, reg_t r2, const char *label);
  void finalize();
};

} // namespace qrisc
