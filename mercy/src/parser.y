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
%token<char *> identifier "identifier";

%token void bool int int8 int16 int32 int64 uint uint8 uint16 uint32 uint64;
%token let return

%token ',' "comma";
%nterm<mercy::ASTNode *>
  declaration
  statement

%nterm<mercy::Expression *>
  additive-expression
  expression
  multiplicative-expression
  prefix-expression
  postfix-expression
  primary-expression
  type-expression
  builtin-type;

%nterm<mercy::NodeList *> statement-list expression-list

%start program

%%

program: translation-unit

translation-unit
    : statement-list { ParserResult = std::unique_ptr<ASTNode>($1); }
    | %empty { ParserResult = std::make_unique<NodeList>(); }

declaration
    : let identifier '=' expression ';' { $$ = new Declaration($2, $4, /*IsRef=*/ false); free($2); }
    | let '&' identifier '=' expression ';' { $$ = new Declaration($3, $5, /*IsRef=*/ true); free($3); }

statement-list
    : statement-list statement { $1->append($2); $$ = $1; }
    | statement { $$ = new NodeList($1); }

statement
    : declaration { $$ = $1; }
    | expression ';' { $$ = $1; }
    //| return expression ';'

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
    | expression { $$ = new NodeList($1); }

primary-expression
    : identifier { $$ = new Identifier($1); free($1); }
    | inum { $$ = new IntegralLiteral($1); }
    | '(' expression ')' { $$ = $2; }
    | type-expression

type-expression: builtin-type

builtin-type
    : void { $$ = new BuiltinTypeExpr(BuiltinType::getVoidTy()); }
    | bool { $$ = new BuiltinTypeExpr(BuiltinType::getBoolTy()); }
    | int   { $$ = new BuiltinTypeExpr(BuiltinType::getIntTy()); }
    | int8  { $$ = new BuiltinTypeExpr(BuiltinType::getInt8Ty()); }
    | int16 { $$ = new BuiltinTypeExpr(BuiltinType::getInt16Ty()); }
    | int32 { $$ = new BuiltinTypeExpr(BuiltinType::getInt32Ty()); }
    | int64 { $$ = new BuiltinTypeExpr(BuiltinType::getInt64Ty()); }
    | uint   { $$ = new BuiltinTypeExpr(BuiltinType::getUintTy()); }
    | uint8  { $$ = new BuiltinTypeExpr(BuiltinType::getUint8Ty()); }
    | uint16 { $$ = new BuiltinTypeExpr(BuiltinType::getUint16Ty()); }
    | uint32 { $$ = new BuiltinTypeExpr(BuiltinType::getUint32Ty()); }
    | uint64 { $$ = new BuiltinTypeExpr(BuiltinType::getUint64Ty()); }

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
