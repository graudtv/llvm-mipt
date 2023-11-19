#include "Assembler.h"
#include <algorithm>
#include <cassert>

using namespace qrisc;

extern int yyparse();
extern FILE *yyin;

Assembler &Assembler::getInstance() {
  static Assembler Asm;
  return Asm;
}

void Assembler::parse(FILE *In) {
  yyin = In;
  while (yyparse() == 0 && !feof(stdin))
    ;
}

void Assembler::print(std::ostream &Os) {
  std::for_each(Instrs.begin(), Instrs.end(), [&Os](Instr I) { I.print(Os); });
}

void Assembler::appendLabel(const char *label) {
  if (Labels.count(label)) {
    std::cerr << "Error: label '" << label << "' redefined\n";
    exit(1);
  }
  Labels[label] = getNextInstrIdx();
}

void Assembler::appendRInstr(opcode_t opcode, reg_t r1, reg_t r2, reg_t r3) {
  Instrs.push_back(Instr::makeRInstr(opcode, r1, r2, r3));
}

void Assembler::appendIInstr(opcode_t opcode, reg_t r1, reg_t r2, imm_t imm) {
  Instrs.push_back(Instr::makeIInstr(opcode, r1, r2, imm));
}

void Assembler::appendIInstr(opcode_t opcode, reg_t r1, reg_t r2,
                             const char *label) {
  UnresolvedLabels.push_back(
      UnresolvedLabel{std::string(label), getNextInstrIdx()});
  Instrs.push_back(Instr::makeIInstr(opcode, r1, r2, 0));
}

void Assembler::finalize() {}
