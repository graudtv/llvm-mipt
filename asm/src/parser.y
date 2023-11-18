%{

#include "Assembler.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

qrisc::Assembler &Asm = qrisc::Assembler::getInstance();

%}

%define parse.error detailed

%union { unsigned opcode; unsigned reg; unsigned imm; const char *label;}

%token TK_COMMA "comma";
%token TK_NEWLINE "newline";
%token<label> TK_LABEL "label";
%token<opcode> TK_R_INSTR "r-instr";
%token<opcode> TK_I_INSTR "i-instr";
%token<imm> TK_IMM "immediate value";
%token<reg> TK_REG "register name";

%start program

%%

program: instr_list                                          { Asm.finalize(); }
instr_list: instr instr_list | %empty;
instr: r_instr | i_instr | label | TK_NEWLINE;

r_instr: TK_R_INSTR TK_REG TK_COMMA TK_REG TK_COMMA TK_REG   { Asm.appendRInstr($1, $2, $4, $6); }
i_instr: TK_I_INSTR TK_REG TK_COMMA TK_REG TK_COMMA TK_IMM   { Asm.appendIInstr($1, $2, $4, $6); }
label: TK_LABEL                                              { Asm.appendLabel($1); }

%%

void yyerror(const char *err) {
  std::cerr << "Error: " << err << "\n";
  exit(1);
}
