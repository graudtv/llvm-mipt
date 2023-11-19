%option noyywrap
%option yylineno

%{
#include <iostream>
#include "parser.tab.h"
#include "Instr.h"

using namespace qrisc;

%}

%%

[ \t]	; /* ignore whitespaces */
#.*     ; /* ingore commets */

[\n]            { return TK_NEWLINE; }
,               { return TK_COMMA; }
:               { return TK_COLON; }

  /* instruction mnemonics */
add|ADD         { yylval.opcode = OPCODE_ADD; return TK_R_INSTR; }
addi|ADDI       { yylval.opcode = OPCODE_ADDI; return TK_I_INSTR; }
and|AND         { yylval.opcode = OPCODE_AND; return TK_R_INSTR; }
andi|ANDI       { yylval.opcode = OPCODE_ANDI; return TK_I_INSTR; }
or|OR           { yylval.opcode = OPCODE_OR; return TK_R_INSTR; }
ori|ORI         { yylval.opcode = OPCODE_ORI; return TK_I_INSTR; }
xor|XOR         { yylval.opcode = OPCODE_XOR; return TK_R_INSTR; }
xori|XORI       { yylval.opcode = OPCODE_XORI; return TK_I_INSTR; }
sub|SUB         { yylval.opcode = OPCODE_SUB; return TK_R_INSTR; }
subi|SUBI       { yylval.opcode = OPCODE_SUBI; return TK_I_INSTR; }
mul|MUL         { yylval.opcode = OPCODE_MUL; return TK_R_INSTR; }
muli|MULI       { yylval.opcode = OPCODE_MULI; return TK_I_INSTR; }
divu|DIVU       { yylval.opcode = OPCODE_DIVU; return TK_R_INSTR; }
diviu|DIVIU     { yylval.opcode = OPCODE_DIVIU; return TK_I_INSTR; }
remu|UREMU      { yylval.opcode = OPCODE_REMU; return TK_R_INSTR; }
remiu|UREMIU    { yylval.opcode = OPCODE_REMIU; return TK_I_INSTR; }
lui|LUI         { yylval.opcode = OPCODE_LUI; return TK_I_INSTR; }
slt|SLT         { yylval.opcode = OPCODE_SLT; return TK_R_INSTR; }
slti|SLTI       { yylval.opcode = OPCODE_SLTI; return TK_I_INSTR; }
sltu|SLTU       { yylval.opcode = OPCODE_SLTU; return TK_R_INSTR; }
sltiu|STLIU     { yylval.opcode = OPCODE_SLTIU; return TK_I_INSTR; }

load|LOAD       { yylval.opcode = OPCODE_LOAD; return TK_I_INSTR; }
store|STORE     { yylval.opcode = OPCODE_STORE; return TK_I_INSTR; }

beq|BEQ         { yylval.opcode = OPCODE_BEQ; return TK_I_INSTR; }
bne|BNE         { yylval.opcode = OPCODE_BNE; return TK_I_INSTR; }
bgt|BGT         { yylval.opcode = OPCODE_BGT; return TK_I_INSTR; }
bge|BGE         { yylval.opcode = OPCODE_BGE; return TK_I_INSTR; }
blt|BLT         { yylval.opcode = OPCODE_BLT; return TK_I_INSTR; }
ble|BLE         { yylval.opcode = OPCODE_BLE; return TK_I_INSTR; }
jalr|JALR       { yylval.opcode = OPCODE_JALR; return TK_I_INSTR; }

  /* numeric literals */
0x[0-9a-fA-F]+  {yylval.imm = strtol(yytext, nullptr, 16); return TK_IMM; }
[-]?[0-9]+      {yylval.imm = atoi(yytext); return TK_IMM; }
  /* ISA registers */
r[0-9]|r[1-2][0-9]|r30|r31   {yylval.reg = atoi(yytext + 1); return TK_REG; }
  /* ABI registers */
zero            { yylval.reg = 0; return TK_REG; }
ra              { yylval.reg = 1; return TK_REG; }
rv0             { yylval.reg = 2; return TK_REG; }
rv1             { yylval.reg = 3; return TK_REG; }
rv2             { yylval.reg = 4; return TK_REG; }
rv3             { yylval.reg = 5; return TK_REG; }
a0              { yylval.reg = 6; return TK_REG; }
a1              { yylval.reg = 7; return TK_REG; }
a2              { yylval.reg = 8; return TK_REG; }
a3              { yylval.reg = 9; return TK_REG; }
a4              { yylval.reg = 10; return TK_REG; }
a5              { yylval.reg = 11; return TK_REG; }
t0              { yylval.reg = 12; return TK_REG; }
t1              { yylval.reg = 13; return TK_REG; }
t2              { yylval.reg = 14; return TK_REG; }
t3              { yylval.reg = 15; return TK_REG; }
t4              { yylval.reg = 17; return TK_REG; }
t5              { yylval.reg = 17; return TK_REG; }
t6              { yylval.reg = 18; return TK_REG; }
t7              { yylval.reg = 19; return TK_REG; }
s0              { yylval.reg = 20; return TK_REG; }
s1              { yylval.reg = 21; return TK_REG; }
s2              { yylval.reg = 22; return TK_REG; }
s3              { yylval.reg = 23; return TK_REG; }
s4              { yylval.reg = 24; return TK_REG; }
s5              { yylval.reg = 25; return TK_REG; }
rsp             { yylval.reg = 29; return TK_REG; }
rbp             { yylval.reg = 30; return TK_REG; }
rpc             { yylval.reg = 31; return TK_REG; }

  /* Identifiers must be handled after everithing else */
[a-zA-Z_.]+     { yylval.id = strdup(yytext); return TK_IDENTIFIER; }

.               {
                  std::cerr << "line " << yylineno << ": unexpected symbol: " << yytext << "\n";
                  return YYerror; 
                }

%%
