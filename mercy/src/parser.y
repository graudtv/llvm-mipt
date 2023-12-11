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

static std::unique_ptr<TranslationUnit> ParserResult;

%}

%define parse.error detailed
%define api.token.prefix {TK_}
%define api.value.type union

%token<int> inum "integer";
%token<char *> identifier "identifier";
%token<char *> str "string literal";
%token LOR "||";
%token LAND "&&";
%token EQ "==";
%token NE "!=";
%token LE "<=";
%token GE ">=";
%token LSHIFT "<<";
%token RSHIFT ">>";

%token void bool int int8 int16 int32 int64 uint uint8 uint16 uint32 uint64 string;
%token let return when otherwise

%token ',' "comma";
%nterm<mercy::ASTNode *>
  declaration
  statement
  function-parameter

%nterm<mercy::Expression *>
  expression
  assignment-expression
  logical-or-expression
  logical-and-expression
  or-expression
  xor-expression
  and-expression
  equality-expression
  relational-expression
  shift-expression
  additive-expression
  multiplicative-expression
  prefix-expression
  postfix-expression
  primary-expression
  numeric-literal
  string-literal
  builtin-type

%nterm<mercy::NodeList *>
  declaration-list
  optional-statement-list
  statement-list
  expression-list
  optional-function-parameter-list
  function-parameter-list

%nterm<mercy::FunctionDomain *> optional-domain

%start program

%%

program: translation-unit
translation-unit
    : declaration-list { ParserResult = std::make_unique<TranslationUnit>($1); }
    | %empty { ParserResult = std::make_unique<TranslationUnit>(); }
declaration-list
    : declaration-list declaration { $1->append($2); $$ = $1; }
    | declaration { $$ = new NodeList($1); }

declaration
    : let identifier '=' expression ';' { $$ = new VariableDecl($2, $4, /*IsRef=*/ false); free($2); }
    | let '&' identifier '=' expression ';' { $$ = new VariableDecl($3, $5, /*IsRef=*/ true); free($3); }
    | let identifier '(' optional-function-parameter-list ')' '{' optional-statement-list '}' optional-domain ';' { $$ = new FunctionFragment($2, $4, $7, $9); free($2); }
    | let identifier '(' optional-function-parameter-list ')' '=' expression optional-domain ';' { $$ = new FunctionFragment($2, $4, new NodeList(new ReturnStmt($7)), $8); free($2); }
optional-function-parameter-list
    : function-parameter-list
    | %empty { $$ = new NodeList; }
function-parameter-list
    : function-parameter-list ',' function-parameter { $1->append($3); $$ = $1; }
    | function-parameter { $$ = new NodeList($1); }
function-parameter
    : identifier { $$ = new FuncParamDecl($1, /*IsRef=*/ false); free($1); }
    | '&' identifier { $$ = new FuncParamDecl($2, /*IsRef=*/ true); free($2); }
optional-statement-list
    : statement-list
    | %empty { $$ = new NodeList; }
statement-list
    : statement-list statement { $1->append($2); $$ = $1; }
    | statement { $$ = new NodeList($1); }
statement
    : declaration { $$ = $1; }
    | expression ';' { $$ = $1; }
    | return expression ';' { $$ = new ReturnStmt($2); }
optional-domain
    : when expression { $$ = new FunctionDomain($2); }
    | otherwise { $$ = new FunctionDomain(/*IsGlobal=*/ false); }
    | %empty { $$ = new FunctionDomain(/*IsGlobal=*/ true); }

expression
    : assignment-expression
assignment-expression
    : postfix-expression '=' assignment-expression { $$ = new BinaryOperator(BinaryOperator::ASSIGN, $1, $3); }
    | logical-or-expression
logical-or-expression
    : logical-or-expression "||" logical-and-expression { $$ = new BinaryOperator(BinaryOperator::LOR, $1, $3); }
    | logical-and-expression
logical-and-expression
    : logical-and-expression "&&" or-expression { $$ = new BinaryOperator(BinaryOperator::LAND, $1, $3); }
    | or-expression
or-expression
    : or-expression '|' xor-expression { $$ = new BinaryOperator(BinaryOperator::OR, $1, $3); }
    | xor-expression
xor-expression
    : xor-expression '^' and-expression { $$ = new BinaryOperator(BinaryOperator::XOR, $1, $3); }
    | and-expression
and-expression
    : and-expression '&' equality-expression { $$ = new BinaryOperator(BinaryOperator::AND, $1, $3); }
    | equality-expression
equality-expression
    : equality-expression "==" relational-expression { $$ = new BinaryOperator(BinaryOperator::EQ, $1, $3); }
    | equality-expression "!=" relational-expression { $$ = new BinaryOperator(BinaryOperator::NE, $1, $3); }
    | relational-expression
relational-expression
    : relational-expression '<' shift-expression { $$ = new BinaryOperator(BinaryOperator::LT, $1, $3); }
    | relational-expression '>' shift-expression { $$ = new BinaryOperator(BinaryOperator::GT, $1, $3); }
    | relational-expression "<=" shift-expression { $$ = new BinaryOperator(BinaryOperator::LE, $1, $3); }
    | relational-expression ">=" shift-expression { $$ = new BinaryOperator(BinaryOperator::GE, $1, $3); }
    | shift-expression
shift-expression
    : shift-expression "<<" additive-expression { $$ = new BinaryOperator(BinaryOperator::LSHIFT, $1, $3); }
    | shift-expression ">>" additive-expression { $$ = new BinaryOperator(BinaryOperator::RSHIFT, $1, $3); }
    | additive-expression
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
    : '!' postfix-expression { $$ = new UnaryOperator(UnaryOperator::NOT, $2); }
    | postfix-expression
postfix-expression
    : postfix-expression '(' ')' { $$ = new FunctionCall($1); }
    | postfix-expression '(' expression-list ')' { $$ = new FunctionCall($1, $3); }
    | postfix-expression '[' expression ']' { $$ = new ArraySubscriptExpr($1, $3); }
    | primary-expression
primary-expression
    : identifier { $$ = new Identifier($1); free($1); }
    | numeric-literal
    | string-literal
    | '(' expression ')' { $$ = $2; }
    | builtin-type
expression-list
    : expression-list ',' expression { $1->append($3); $$ = $1; }
    | expression { $$ = new NodeList($1); }

numeric-literal: inum { $$ = new IntegralLiteral($1); }
string-literal: str { $$ = new StringLiteral($1); free($1); }

builtin-type
    : void { $$ = new TypeExpr(BuiltinType::getVoidTy()); }
    | bool { $$ = new TypeExpr(BuiltinType::getBoolTy()); }
    | int   { $$ = new TypeExpr(BuiltinType::getIntTy()); }
    | int8  { $$ = new TypeExpr(BuiltinType::getInt8Ty()); }
    | int16 { $$ = new TypeExpr(BuiltinType::getInt16Ty()); }
    | int32 { $$ = new TypeExpr(BuiltinType::getInt32Ty()); }
    | int64 { $$ = new TypeExpr(BuiltinType::getInt64Ty()); }
    | uint   { $$ = new TypeExpr(BuiltinType::getUintTy()); }
    | uint8  { $$ = new TypeExpr(BuiltinType::getUint8Ty()); }
    | uint16 { $$ = new TypeExpr(BuiltinType::getUint16Ty()); }
    | uint32 { $$ = new TypeExpr(BuiltinType::getUint32Ty()); }
    | uint64 { $$ = new TypeExpr(BuiltinType::getUint64Ty()); }
    | string { $$ = new TypeExpr(BuiltinType::getStringTy()); }

%%

void yyerror(const char *err) {
  llvm::errs() << "line " << yylineno << ": " << err << "\n";
  exit(1);
}

std::unique_ptr<TranslationUnit> mercy::parse(FILE *In) {
  yyin = In;
  if (yyparse() != 0) {
    llvm::errs() << "Unknown parser error\n";
    exit(1);
  }
  return std::move(ParserResult);
}
