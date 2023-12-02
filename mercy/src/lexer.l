%option noyywrap
%option yylineno

%{
namespace mercy { class ASTNode; class ExpressionList; }

#include "parser.tab.h"
%}

%%
[/][/].*\n      ; // comment
[ \t\r\n]      ; // whitespace

  /* numeric literals */
0x[0-9a-fA-F]+  {yylval.TK_inum = strtol(yytext, nullptr, 16); return TK_inum; }
0[0-9]+         {yylval.TK_inum = strtol(yytext, nullptr, 8); return TK_inum; }
[-]?[0-9]+      {yylval.TK_inum = atoi(yytext); return TK_inum; }

  /* identifier */
[a-zA-Z_.]+     { yylval.TK_identifier = yytext; return TK_identifier; }

  /* anything else */
.               { return *yytext; }


%%
