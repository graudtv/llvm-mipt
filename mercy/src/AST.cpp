#include "AST.h"
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>

using namespace mercy;

namespace {

struct tabulate {
  unsigned Shift;
  tabulate(unsigned S) : Shift(S) {}
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, const tabulate &T) {
  for (unsigned I = 0; I < T.Shift; ++I)
    Os << "  ";
  return Os;
}

const char *getBinaryOpKindStr(BinaryOperator::BinOpKind Kind) {
  // clang-format off
  switch (Kind) {
  case BinaryOperator::ASSIGN: return "=";
  case BinaryOperator::LOR: return "||";
  case BinaryOperator::LAND: return "&&";
  case BinaryOperator::OR: return "|";
  case BinaryOperator::XOR: return "^";
  case BinaryOperator::AND: return "&";
  case BinaryOperator::EQ: return "==";
  case BinaryOperator::NE: return "!=";
  case BinaryOperator::LT: return "<";
  case BinaryOperator::GT: return ">";
  case BinaryOperator::LE: return "<=";
  case BinaryOperator::GE: return ">=";
  case BinaryOperator::LSHIFT: return "<<";
  case BinaryOperator::RSHIFT: return ">>";
  case BinaryOperator::ADD: return "+";
  case BinaryOperator::SUB: return "-";
  case BinaryOperator::MUL: return "*";
  case BinaryOperator::DIV: return "/";
  case BinaryOperator::REM: return "%";
  }
  // clang-format on
  llvm_unreachable("Invalid binary operator kind");
}

const char *getUnaryOpKindStr(UnaryOperator::UnaryOpKind Kind) {
  switch (Kind) {
  case UnaryOperator::NOT:
    return "!";
  }
  llvm_unreachable("Invalid unary operator kind");
}

} // namespace

bool Expression::isLValue() const {
  return llvm::isa<Identifier>(this) || llvm::isa<ArraySubscriptExpr>(this);
}

void IntegralLiteral::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "IntegralLiteral " << Value << '\n';
}

IntegralLiteral *IntegralLiteral::clone() const {
  return new IntegralLiteral(Value);
}

void StringLiteral::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "StringLiteral '" << Value << "'\n";
}

StringLiteral *StringLiteral::clone() const {
  return new StringLiteral(Value);
}

void TypeExpr::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "TypeExpr '" << *GValue << "'\n";
}

TypeExpr *TypeExpr::clone() const {
  return new TypeExpr(GValue);
}

const char *BinaryOperator::getMnemonic() const {
  return getBinaryOpKindStr(Kind);
}

void BinaryOperator::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "BinaryOperator '" << getMnemonic() << "'\n";
  LHS->print(Os, Shift + 1);
  RHS->print(Os, Shift + 1);
}

BinaryOperator *BinaryOperator::clone() const {
  return new BinaryOperator(Kind, LHS->clone(), RHS->clone());
}

void UnaryOperator::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "UnaryOperator '" << getUnaryOpKindStr(Kind)
     << "'\n";
  Expr->print(Os, Shift + 1);
}

UnaryOperator *UnaryOperator::clone() const {
  return new UnaryOperator(Kind, Expr->clone());
}

void Identifier::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "Identifier '" << Name << "'\n";
}

Identifier *Identifier::clone() const { return new Identifier(Name); }

void NodeList::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "NodeList" << (isEmpty() ? ": {}\n" : "\n");
  llvm::for_each(Nodes, [&Os, Shift](auto &&N) { N->print(Os, Shift + 1); });
}

NodeList *NodeList::clone() const {
  NodeList *List = new NodeList;
  for (auto &&N : Nodes)
    List->append(N->clone());
  return List;
}

void VariableDecl::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "VariableDecl " << ((IsRef) ? "& " : "") << "'"
     << getId() << "'\n";
  if (getInitializer())
    getInitializer()->print(Os, Shift + 1);
}

VariableDecl *VariableDecl::clone() const {
  return new VariableDecl(getId(), getInitializer()->clone(), IsRef);
}

void FuncParamDecl::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "FuncParamDecl " << ((IsRef) ? "& " : "") << "'"
     << getId() << "'\n";
}

FuncParamDecl *FuncParamDecl::clone() const {
  return new FuncParamDecl(getId(), IsRef);
}

void FunctionDomain::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "FunctionDomain";
  if (isGlobal())
    Os << ": global\n";
  else if (isOtherwise())
    Os << ": otherwise\n";
  else {
    Os << '\n';
    Expr->print(Os, Shift + 1);
  }
}

FunctionDomain *FunctionDomain::clone() const {
  if (isCustom())
    return new FunctionDomain(Expr->clone());
  return new FunctionDomain(isGlobal());
}

void FunctionFragment::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "FunctionFragment '" << getId() << "'\n";
  Domain->print(Os, Shift + 1);
  Params->print(Os, Shift + 1);
  Body->print(Os, Shift + 1);
}

FunctionFragment *FunctionFragment::clone() const {
  return new FunctionFragment(getId(), Params->clone(), Body->clone(), Domain->clone());
}

void FunctionDecl::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "FunctionDecl '" << getId() << "'\n";
  llvm::for_each(Fragments,
                 [&Os, Shift](auto &&F) { F->print(Os, Shift + 1); });
}

FunctionDecl *FunctionDecl::clone() const {
  FunctionDecl *Decl = new FunctionDecl(Fragments.front()->clone());
  std::for_each(std::next(Fragments.begin()), Fragments.end(),
                [Decl](auto &&F) { Decl->appendFragment(F->clone()); });
  return Decl;
}

void FunctionCall::print(llvm::raw_ostream &Os, unsigned Shift) const {
  if (auto *Id = llvm::dyn_cast<Identifier>(Callee.get()))
    Os << tabulate(Shift) << "FunctionCall '" << Id->getName() << "'\n";
  else
    Callee->print(Os, Shift + 1);
  if (getArgCount() > 0)
    Args->print(Os, Shift + 1);
}

FunctionCall *FunctionCall::clone() const {
  return new FunctionCall(Callee->clone(), Args->clone());
}

void ArraySubscriptExpr::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "ArraySubscriptExpr\n";
  Array->print(Os, Shift + 1);
  Index->print(Os, Shift + 1);
}

ArraySubscriptExpr *ArraySubscriptExpr::clone() const {
  return new ArraySubscriptExpr(Array->clone(), Index->clone());
}

void ReturnStmt::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "ReturnStmt\n";
  Expr->print(Os, Shift + 1);
}

ReturnStmt *ReturnStmt::clone() const {
  return new ReturnStmt(Expr->clone());
}

void TranslationUnit::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "TranslationUnit\n";
  Decls->print(Os, Shift + 1);
}

TranslationUnit *TranslationUnit::clone() const {
  return new TranslationUnit(Decls->clone());
}
