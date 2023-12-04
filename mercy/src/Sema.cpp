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
VISIT_NODE(VariableDecl)
VISIT_NODE(FunctionDecl)
VISIT_NODE(FuncParamDecl)
VISIT_NODE(Identifier)

SKIP_NODE(IntegralLiteral)

UNREACHABLE_NODE(BuiltinTypeExpr)
UNREACHABLE_NODE(NodeList)

namespace {
void emitError(ASTNode *Node, const llvm::Twine &T) {
  llvm::errs() << "line " << Node->getLocation().LineNo << ": " << T << '\n';
  exit(1);
}
} // namespace

Declaration *Sema::findDecl(const std::string &Id) {
  for (auto ScopeIt = Scopes.rbegin(); ScopeIt != Scopes.rend(); ++ScopeIt)
    if (auto DeclIt = ScopeIt->find(Id); DeclIt != ScopeIt->end())
      return DeclIt->second;
  return nullptr;
}

void Sema::insertDecl(Declaration *Decl) {
  Scope &CurScope = Scopes.back();
  if (auto It = CurScope.find(Decl->getId()); It != CurScope.end())
    emitError(Decl, "redefinition of '" + Decl->getId() + "'");
  CurScope.insert(std::make_pair(Decl->getId(), Decl));
}

FunctionDecl *
Sema::getOrCreateFunctionInstance(FunctionDecl *FD,
                                  llvm::ArrayRef<Type *> ParamTys) {
  InstanceList &Instances = FunctionInstances[FD];
  auto It = llvm::find_if(Instances, [ParamTys](auto &&I) {
    return llvm::equal(I->getParamTypes(), ParamTys);
  });
  if (It != Instances.end())
    return It->get();

  /* Instantiate function */
  unsigned InstanceId = Instances.size();
  FunctionDecl *Instance = FD->clone();
  Instances.emplace_back(Instance);

  Instance->addIdPrefix("__" + std::to_string(InstanceId) + "_");
  for (size_t I = 0; I < ParamTys.size(); ++I)
    Instance->getParam(I)->setType(ParamTys[I]);

  pushScope();
  llvm::for_each(Instance->getParams(),
                 [this](FuncParamDecl *D) { D->sema(*this); });
  Instance->getInitializer()->sema(*this);
  popScope();
  return Instance;
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
  BinaryOperator::BinOpKind BK = BinOp->getKind();
  bool isCmp = (BK == BinaryOperator::EQ || BK == BinaryOperator::NE ||
                BK == BinaryOperator::LT || BK == BinaryOperator::GT ||
                BK == BinaryOperator::LE || BK == BinaryOperator::GE);
  BinOp->setType(isCmp ? BuiltinType::getBoolTy() : OperandType);
}

void Sema::actOnUnaryOperator(UnaryOperator *Op) {
  Expression *Expr = Op->getExpr();
  Expr->sema(*this);
  if (Op->getKind() == UnaryOperator::NOT) {
    if (Expr->getType() != BuiltinType::getBoolTy())
      emitError(Op, "invalid operand in '!' operator, must be boolean");
    Op->setType(Expr->getType());
    return;
  }
  llvm_unreachable("unhandled unary operator kind");
}

void Sema::actOnFunctionCall(FunctionCall *FC) {
  ASTNode *Callee = FC->getCallee();
  std::vector<Type *> ArgTys;

  for (Expression *Arg : FC->getArgs()) {
    Arg->sema(*this);
    ArgTys.push_back(Arg->getType());
  }

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
  if (Identifier *Id = llvm::dyn_cast<Identifier>(Callee)) {
    if (Id->getName() == "print") {
      FC->setType(BuiltinType::getVoidTy());
      return;
    }
    Declaration *Decl = findDecl(Id->getName());
    if (!Decl)
      emitError(Callee, "call to undeclared function '" + Id->getName() + "'");
    assert(llvm::isa<FunctionDecl>(Decl) && "reference calls not implemented");
    FunctionDecl *FD = llvm::cast<FunctionDecl>(Decl);
    FunctionDecl *Instance = getOrCreateFunctionInstance(FD, ArgTys);
    FC->setCalleeDecl(Instance);
    FC->setType(Instance->getInitializer()->getType());
    return;
  };
  assert(0 && "not implemented: cannot handle call");
}

/* Insert declaration into the current scope */
void Sema::actOnVariableDecl(VariableDecl *Decl) {
  insertDecl(Decl);
  Decl->getInitializer()->sema(*this);
}

void Sema::actOnFunctionDecl(FunctionDecl *Decl) { insertDecl(Decl); }
void Sema::actOnFuncParamDecl(FuncParamDecl *Decl) { insertDecl(Decl); }

/* Lookup declaration and set type of identifier */
void Sema::actOnIdentifier(Identifier *Id) {
  Declaration *Decl = findDecl(Id->getName());
  if (!Decl)
    emitError(Id, "use of undeclared identifier '" + Id->getName() + "'");
  Id->setDeclaration(Decl);
  Id->setType(Decl->getType());
}

void Sema::run(ASTNode *TU) {
  Scopes.emplace_back();
  auto Statements = llvm::cast<NodeList>(TU)->getNodes();
  for (auto S : Statements)
    S->sema(*this);
  Scopes.pop_back();
  assert(Scopes.empty() && "bug in scope processing");

  for (auto &&[Decl, Instances] : FunctionInstances)
    for (auto &&I : Instances)
      llvm::cast<NodeList>(TU)->append(I.release());
}
