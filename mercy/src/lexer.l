%option noyywrap
%option yylineno

%{
namespace mercy { class ASTNode; class NodeList; class Expression; }

#include "parser.tab.h"
%}

%%
[/][/].*\n      ; // comment
[ \t\r\n]      ; // whitespace

  /* numeric literals */
0x[0-9a-fA-F]+  {yylval.TK_inum = strtol(yytext, nullptr, 16); return TK_inum; }
0[0-9]+         {yylval.TK_inum = strtol(yytext, nullptr, 8); return TK_inum; }
[-]?[0-9]+      {yylval.TK_inum = atoi(yytext); return TK_inum; }

  /* multicharacter operators */
"<="              { return TK_LE; }
">="              { return TK_GE; }
"<<"              { return TK_LSHIFT; }
">>"              { return TK_RSHIFT; }

  /* builtin types */
void            { return TK_void; }
bool            { return TK_bool; }
int             { return TK_int; }
int8            { return TK_int8; }
int16           { return TK_int16; }
int32           { return TK_int32; }
int64           { return TK_int64; }
uint            { return TK_uint; }
uint8           { return TK_uint8; }
uint16          { return TK_uint16; }
uint32          { return TK_uint32; }
uint64          { return TK_uint64; }
  /* Other keywords */
let             { return TK_let; }
return          { return TK_return; }

  /* identifier */
[a-zA-Z_.]+     { yylval.TK_identifier = strdup(yytext); return TK_identifier; }

  /* anything else */
.               { return *yytext; }


%%
