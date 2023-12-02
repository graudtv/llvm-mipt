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

void BinaryOperator::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "BinaryOperator '" << getBinaryOpKindStr(Kind)
     << "'\n";
  LHS->print(Os, Shift + 1);
  RHS->print(Os, Shift + 1);
}

void UnaryOperator::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "UnaryOperator '" << getUnaryOpKindStr(Kind)
     << "'\n";
  Expr->print(Os, Shift + 1);
}

void Identifier::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "Identifier '" << Name << "'\n";
}

void ExpressionList::print(llvm::raw_ostream &Os, unsigned Shift) const {
  Os << tabulate(Shift) << "ExpressionList\n";
  llvm::for_each(Exprs,
                 [&Os, Shift](auto &&Expr) { Expr->print(Os, Shift + 1); });
}

void FunctionCall::print(llvm::raw_ostream &Os, unsigned Shift) const {
  if (auto *Id = llvm::dyn_cast<Identifier>(Callee.get()))
    Os << tabulate(Shift) << "FunctionCall '" << Id->getName() << "'\n";
  else
    Callee->print(Os, Shift + 1);
  if (getArgCount() > 0)
    Args->print(Os, Shift + 1);
}
