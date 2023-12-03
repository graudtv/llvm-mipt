#include "Sema.h"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

using namespace mercy;

#define VISIT_NODE(NodeName)                                                   \
  void NodeName::sema(Sema &S) { S.actOn##NodeName(this); }

#define SKIP_NODE(NodeName)                                                    \
  void NodeName::sema(Sema &S) {}

#define UNREACHABLE_NODE(NodeName)                                             \
  void NodeName::sema(Sema &S) { llvm_unreachable("unreachable node reached"); }

VISIT_NODE(BinaryOperator)
VISIT_NODE(UnaryOperator)
VISIT_NODE(FunctionCall)
VISIT_NODE(Declaration)
VISIT_NODE(FunctionDeclaration)
VISIT_NODE(Identifier)

SKIP_NODE(IntegralLiteral)

UNREACHABLE_NODE(BuiltinTypeExpr)
UNREACHABLE_NODE(NodeList)

void emitError(ASTNode *Node, const llvm::Twine &T) {
  llvm::errs() << "line " << Node->getLocation().LineNo << ": " << T << '\n';
  exit(1);
}

void Sema::actOnBinaryOperator(BinaryOperator *BinOp) {
  BinOp->getLHS()->sema(*this);
  BinOp->getRHS()->sema(*this);

  Expression *LHS = BinOp->getLHS();
  Expression *RHS = BinOp->getRHS();
  if (!LHS->getType()->isBuiltinType())
    emitError(LHS, "invalid left operand in binary operator");
  if (!RHS->getType()->isBuiltinType())
    emitError(RHS, "invalid right operand in binary operator");
  if (LHS->getType() != RHS->getType())
    emitError(LHS, "operands of binary operator have different types");
  BuiltinType *OperandType = llvm::cast<BuiltinType>(LHS->getType());
  if (!OperandType->isInteger())
    emitError(BinOp, "invalid operand in binary operator, must be integer");
  BinOp->setType(OperandType);
}

void Sema::actOnUnaryOperator(UnaryOperator *Op) {
  Expression *Expr = Op->getExpr();
  if (Op->getKind() == UnaryOperator::NEG) {
    if (Expr->getType() != BuiltinType::getBoolTy())
      emitError(Op, "invalid operand in negation, must be boolean");
    Op->setType(Expr->getType());
    return;
  }
  llvm_unreachable("unhandled unary operator kind");
}

void Sema::actOnFunctionCall(FunctionCall *FC) {
  ASTNode *Callee = FC->getCallee();
  for (Expression *Arg : FC->getArgs())
    Arg->sema(*this);

  /* Handle type conversions */
  if (auto *BTE = llvm::dyn_cast<BuiltinTypeExpr>(Callee)) {
    /* Conversion to builtin type */
    if (auto *CastTy = llvm::dyn_cast<BuiltinType>(BTE->getReferencedType())) {
      if (FC->getArgCount() != 1) {
        emitError(Callee, "invalid number of arguments in cast to '" +
                              CastTy->toString() + "'");
      }
      auto *ArgTy = llvm::dyn_cast<BuiltinType>(FC->getArg(0)->getType());
      if (!ArgTy->isBuiltinType() ||
          !llvm::cast<BuiltinType>(ArgTy)->isInteger() || !CastTy->isInteger())
        emitError(Callee, "cast from '" + ArgTy->toString() + "' to '" +
                              CastTy->toString() + "' is not allowed");

      FC->setType(CastTy);
      return;
    }
    // TODO: array constructor
    assert(0 && "unimplemented or illegal cast");
  }
  // TODO: handle function calls
  FC->setType(BuiltinType::getVoidTy());
}

/* Insert declaration into the current scope */
void Sema::actOnDeclaration(Declaration *Decl) {
  if (auto It = Decls.find(Decl->getId()); It != Decls.end())
    emitError(Decl, "redefinition of '" + Decl->getId() + "'");
  Decls[Decl->getId()] = Decl;
  Decl->getInitializer()->sema(*this);
}

void Sema::actOnFunctionDeclaration(FunctionDeclaration *FD) {

}

/* Lookup declaration and set type of identifier */
void Sema::actOnIdentifier(Identifier *Id) {
  auto It = Decls.find(Id->getName());
  if (It == Decls.end())
    emitError(Id, "use of undeclared identifier '" + Id->getName() + "'");
  Declaration *Decl = It->second;
  Id->setDeclaration(Decl);
  Id->setType(Decl->getInitializer()->getType());
}

void Sema::run(ASTNode *TU) {
  auto Statements = llvm::cast<NodeList>(TU)->getNodes();
  for (auto S : Statements)
    S->sema(*this);
}
