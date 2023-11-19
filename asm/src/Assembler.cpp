#include "Assembler.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace qrisc;

extern int yyparse();
extern FILE *yyin;

Assembler &Assembler::getInstance() {
  static Assembler Asm;
  return Asm;
}

void Assembler::parse(FILE *In) {
  yyin = In;
  while (yyparse() == 0 && !feof(In))
    ;
}

void Assembler::parseFile(const std::string &Filename) {
  FILE *In = fopen(Filename.c_str(), "r");
  if (!In) {
    int Err = errno;
    std::cerr << "Failed to open file '" << Filename << "': " << strerror(Err)
              << std::endl;
    exit(1);
  }
  parse(In);
  fclose(In);
}

void Assembler::parseFileOrStdin(const std::string &Filename) {
  if (Filename.empty() || Filename == "-")
    parse(stdin);
  else
    parseFile(Filename);
}

void Assembler::write(std::ostream &Os) const {
  Os.write(reinterpret_cast<const char *>(Instrs.data()),
           Instrs.size() * sizeof(Instr));
}

void Assembler::print(std::ostream &Os) const {
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

void Assembler::finalize() {
  for (auto &UL : UnresolvedLabels) {
    Instr &User = Instrs[UL.InstrIdx];
    auto It = Labels.find(UL.Label);
    if (It == Labels.end()) {
      std::cerr << "Error: in instruction " << User.getMnemonic()
                << ": undefined label '" << UL.Label << "'" << std::endl;
      exit(1);
    }
    unsigned Imm = It->second - UL.InstrIdx;
    User = Instr::makeIInstr(User.getOpcode(), User.r1(), User.r2(), Imm);
  }
}
