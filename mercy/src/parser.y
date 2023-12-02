%{

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <llvm/Support/raw_ostream.h>
#include "AST.h"
#include "Parser.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(const char* s);

using namespace mercy;

static std::unique_ptr<ASTNode> ParserResult;

%}

%define parse.error detailed
%define api.token.prefix {TK_}
%define api.value.type union

%token<int> inum "integer";
%token<const char *> identifier "identifier";
%token ',' "comma";
%nterm<mercy::ASTNode *>
  additive-expression
  expression
  multiplicative-expression
  prefix-expression
  postfix-expression
  primary-expression;

%nterm<mercy::ExpressionList *> expression-list

%start program

%%

program: expression { ParserResult = std::unique_ptr<ASTNode>($1); }

expression
    : additive-expression

additive-expression
    : additive-expression '+' multiplicative-expression { $$ = new BinaryOperator(BinaryOperator::ADD, $1, $3); }
    | additive-expression '-' multiplicative-expression { $$ = new BinaryOperator(BinaryOperator::SUB, $1, $3); }
    | multiplicative-expression

multiplicative-expression
    : multiplicative-expression '*' prefix-expression { $$ = new BinaryOperator(BinaryOperator::MUL, $1, $3); }
    | multiplicative-expression '/' prefix-expression { $$ = new BinaryOperator(BinaryOperator::DIV, $1, $3); }
    | multiplicative-expression '%' prefix-expression { $$ = new BinaryOperator(BinaryOperator::REM, $1, $3); }
    | prefix-expression

prefix-expression
    : '!' postfix-expression { $$ = new UnaryOperator(UnaryOperator::NEG, $2); }
    | postfix-expression

postfix-expression
    : primary-expression '(' ')' { $$ = new FunctionCall($1); }
    | primary-expression '(' expression-list ')' { $$ = new FunctionCall($1, $3); }
    | primary-expression

expression-list
    : expression-list ',' expression { $1->append($3); $$ = $1; }
    | expression { $$ = new ExpressionList($1); }

primary-expression
    : identifier { $$ = new Identifier($1); }
    | inum { $$ = new IntegralLiteral($1); }
    | '(' expression ')' { $$ = $2; }

%%

void yyerror(const char *err) {
  llvm::errs() << "line " << yylineno << ": " << err << "\n";
  exit(1);
}

std::unique_ptr<ASTNode> mercy::parse(FILE *In) {
  yyin = In;
  if (yyparse() != 0) {
    llvm::errs() << "Unknown parser error\n";
    exit(1);
  }
  return std::move(ParserResult);
}
