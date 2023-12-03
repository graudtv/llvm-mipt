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
  switch (Kind) {
  case BinaryOperator::ADD:
    return "+";
  case BinaryOperator::SUB:
    return "-";
  case BinaryOperator::MUL:
    return "*";
  case BinaryOperator::DIV:
    return "/";
  case BinaryOperator::REM:
    return "%";
  }
  llvm_unreachable("Invalid binary operator kind");
}

const char *getUnaryOpKindStr(UnaryOperator::UnaryOpKind Kind) {
  switch (Kind) {
  case UnaryOperator::NEG:
    return "!";
  }
  llvm_unreachable("Invalid unary operator kind");
}

} // namespace

void IntegralLiteral::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "IntegralLiteral " << Value << '\n';
}

IntegralLiteral *IntegralLiteral::clone() const {
  return new IntegralLiteral(Value);
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

void Declaration::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "Declaration " << ((IsRef) ? "& " : "") << "'"
     << Identifier << "'\n";
  if (Initializer)
    Initializer->print(Os, Shift + 1);
}

Declaration *Declaration::clone() const {
  return new Declaration(Identifier, Initializer->clone(), IsRef);
}

void NodeList::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "NodeList\n";
  llvm::for_each(Nodes, [&Os, Shift](auto &&N) { N->print(Os, Shift + 1); });
}

NodeList *NodeList::clone() const {
  NodeList *List = new NodeList;
  for (auto &&N : Nodes)
    List->append(N->clone());
  return List;
}

void FunctionDeclaration::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "FunctionDeclaration '" << Identifier << "'\n";
  Params->print(Os, Shift + 1);
  Body->print(Os, Shift + 1);
}

FunctionDeclaration *FunctionDeclaration::clone() const {
  return new FunctionDeclaration(Identifier, Params->clone(), Body->clone());
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

void BuiltinTypeExpr::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "BuiltinTypeExpr '";
  getType()->print(Os);
  Os << "'\n";
}

BuiltinTypeExpr *BuiltinTypeExpr::clone() const {
  return new BuiltinTypeExpr(getReferencedType());
}
