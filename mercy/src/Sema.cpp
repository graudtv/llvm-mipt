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
VISIT_NODE(ArraySubscriptExpr)
VISIT_NODE(VariableDecl)
VISIT_NODE(FunctionFragment)
VISIT_NODE(FuncParamDecl)
VISIT_NODE(Identifier)
VISIT_NODE(ReturnStmt)
VISIT_NODE(TranslationUnit)

SKIP_NODE(IntegralLiteral)
SKIP_NODE(TypeExpr)

UNREACHABLE_NODE(NodeList)
UNREACHABLE_NODE(FunctionDecl)
UNREACHABLE_NODE(FunctionDomain)

namespace {
void emitError(const llvm::Twine &T) {
  llvm::errs() << "error: " << T << '\n';
  exit(1);
}
void emitError(ASTNode *Node, const llvm::Twine &T) {
  llvm::errs() << "line " << Node->getLocation().LineNo << ": " << T << '\n';
  exit(1);
}

/* Search for declaration in all enclosing scopes */
Declaration *findDeclRecursive(Scope *Top, const std::string &Id) {
  for (Scope *S = Top; S; S = S->getPrev())
    if (Declaration *Decl = S->find(Id))
      return Decl;
  return nullptr;
}

Scope *findScopeOfDecl(Scope *Top, Declaration *D) {
  for (Scope *S = Top; S; S = S->getPrev())
    if (S->contains(D))
      return S;
  return nullptr;
}

std::string makeFullScopeName(Scope *Top) {
  return Top->getPrev()
             ? (makeFullScopeName(Top->getPrev()) + Top->getName() + ".")
             : (Top->getName() + ".");
}

} // namespace

Declaration *Sema::findDecl(const std::string &Id) {
  return findDeclRecursive(CurScope, Id);
}

void Sema::insertDecl(Declaration *Decl) {
  if (Declaration *PrevDecl = CurScope->find(Decl->getId()))
    emitError(Decl, "redefinition of '" + Decl->getId() + "'");
  CurScope->insert(Decl);
}

TemplateInstance *
Sema::getOrCreateFunctionInstance(FunctionDecl *FD,
                                  llvm::ArrayRef<Type *> ParamTys) {
  InstanceList &Instances = FunctionInstances[FD];
  auto It = llvm::find_if(Instances, [ParamTys](auto &&I) {
    return llvm::equal(I->getDecl()->getParamTypes(), ParamTys);
  });
  if (It != Instances.end()) {
    TemplateInstance *Ins = *It;
    if (!Ins->getDecl()->getReturnType())
      emitError(Ins->getDecl(),
                "failed to deduce template function return type");
    return Ins;
  }

  /* Instantiate function */
  std::string InstanceId = std::to_string(Instances.size());
  TemplateInstance *Instance = new TemplateInstance(FD->clone());
  Instances.push_back(Instance);
  AllInstances.emplace_back(Instance);

  Scope *FDScope = findScopeOfDecl(CurScope, FD);
  assert(FDScope && "scopes are broken");

  Scope *PrevScope = CurScope;

  Instance->addIdPrefix(makeFullScopeName(FDScope));
  Instance->addIdPostfix("." + InstanceId);
  for (FunctionFragment *Fragment : Instance->getDecl()->getFragments()) {
    for (size_t I = 0; I < ParamTys.size(); ++I)
      Fragment->getParam(I)->setType(ParamTys[I]);

    /* Jump back to the scope there function was declared */
    Scope FunctionScope{FDScope, FD->getId() + "." + InstanceId};
    CurScope = &FunctionScope;

    llvm::for_each(Fragment->getParams(),
                   [this](FuncParamDecl *D) { D->sema(*this); });
    if (Fragment->getDomain()->isCustom())
      Fragment->getDomain()->getExpr()->sema(*this);
    actOnFunctionBody(Fragment);
    if (Fragment->getReturnType() != Instance->getDecl()->getReturnType()) {
      emitError(
          FD,
          "function returns values of different types in different domains: '" +
              Fragment->getReturnType()->toString() +
              "' vs previously used type '" +
              Instance->getDecl()->getReturnType()->toString());
    }
  }
  CurScope = PrevScope;
  return Instance;
}

void Sema::actOnFunctionBody(FunctionFragment *Fragment) {
  // TODO: push scope
  for (ASTNode *Stmt : Fragment->getBody())
    Stmt->sema(*this);

  /* Verify return statements and deduce block return type */
  Type *RetTy = nullptr;
  for (ASTNode *Stmt : Fragment->getBody()) {
    if (ReturnStmt *Ret = llvm::dyn_cast<ReturnStmt>(Stmt)) {
      if (RetTy && RetTy != Ret->getRetType())
        emitError(
            Ret,
            llvm::Twine{"mismatching return values in return statements: '"} +
                Ret->getRetType()->toString() + "' vs previously returned '" +
                RetTy->toString());
      if (!RetTy)
        RetTy = Ret->getRetType();
    }
  }
  Fragment->setReturnType(RetTy ? RetTy : BuiltinType::getVoidTy());
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
  BinaryOperator::BinOpKind BK = BinOp->getKind();
  bool IsLogicalOp = (BK == BinaryOperator::LOR || BK == BinaryOperator::LAND);
  bool IsBitOp = (BK == BinaryOperator::OR || BK == BinaryOperator::XOR ||
                  BK == BinaryOperator::AND);
  bool IsCmp = (BK == BinaryOperator::EQ || BK == BinaryOperator::NE ||
                BK == BinaryOperator::LT || BK == BinaryOperator::GT ||
                BK == BinaryOperator::LE || BK == BinaryOperator::GE);
  bool IsAssignment = (BK == BinaryOperator::ASSIGN);
  if (IsAssignment && !LHS->isLValue())
    emitError(LHS, "expression is not assignable");
  if (IsLogicalOp && !OperandType->isBool())
    emitError(BinOp, llvm::Twine{"invalid operand types for operator '"} +
                         BinOp->getMnemonic() + "', must be booleans");
  if (!IsLogicalOp && !OperandType->isInteger())
    emitError(BinOp, llvm::Twine{"invalid operand types for operator '"} +
                         BinOp->getMnemonic() + "', must be integers");
  if (IsBitOp && !OperandType->isUnsigned()) {
    emitError(BinOp, llvm::Twine{"invalid operand types for operator '"} +
                         BinOp->getMnemonic() + "', must be unsigned");
  }
  BinOp->setType((IsCmp || IsLogicalOp) ? BuiltinType::getBoolTy()
                                        : OperandType);
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
  Expression *Callee = FC->getCallee();
  std::vector<Type *> ArgTys;

  for (Expression *Arg : FC->getArgs()) {
    Arg->sema(*this);
    ArgTys.push_back(Arg->getType());
  }

  /* Handle type conversions */
  if (auto *TE = llvm::dyn_cast<TypeExpr>(Callee)) {
    /* Conversion to builtin type */
    if (auto *CastTy = llvm::dyn_cast<BuiltinType>(TE->getValue())) {
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
    emitError(Callee, "type '" + TE->getValue()->toString() +
                          "' cannot be used as function name");
  }
  if (Identifier *Id = llvm::dyn_cast<Identifier>(Callee)) {
    if (Id->getName() == "array") {
      if (!FC->getArgCount())
        emitError(Callee, "array() requires at least one argument");
      Type *ElemTy = ArgTys.front();
      bool AllSame =
          llvm::all_of(ArgTys, [ElemTy](Type *Ty) { return Ty == ElemTy; });
      if (!AllSame) {
        emitError(Callee,
                  "arguments of different types in array() are not allowed");
      }
      FC->setType(ArrayType::get(ElemTy));
      return;
    }
    if (Id->getName() == "alloca") {
      if (FC->getArgCount() != 2)
        emitError(Callee, "invalid number of arguments in alloca()");
      assert(llvm::isa<TypeExpr>(FC->getArg(0)) && "indirect types are not implemented");
      Type *ElemTy = llvm::cast<TypeExpr>(FC->getArg(0))->getValue();
      if(!ElemTy->isBuiltinType() || ElemTy == BuiltinType::getVoidTy())
        emitError(Callee, "illegal or not implemented array type");
      FC->setType(ArrayType::get(ElemTy));
      return;
    }
    if (Id->getName() == "print") {
      FC->setType(BuiltinType::getVoidTy());
      return;
    }
    Declaration *Decl = findDecl(Id->getName());
    if (!Decl)
      emitError(Callee, "call to undeclared function '" + Id->getName() + "'");
    assert(llvm::isa<FunctionDecl>(Decl) && "reference calls not implemented");
    FunctionDecl *FD = llvm::cast<FunctionDecl>(Decl);
    TemplateInstance *Instance = getOrCreateFunctionInstance(FD, ArgTys);
    FC->setCalleeFunc(Instance);
    FC->setType(Instance->getDecl()->getReturnType());
    return;
  };
  assert(0 && "not implemented: cannot handle call");
}

void Sema::actOnArraySubscriptExpr(ArraySubscriptExpr *Expr) {
  Expression *Arr = Expr->getArray();
  Expression *Idx = Expr->getIndex();
  Arr->sema(*this);
  Idx->sema(*this);

  if (!Arr->getType()->isArrayType()) {
    emitError(Arr, "expression of type '" + Arr->getType()->toString() +
                       "' is not an array");
  }
  if (!Idx->getType()->isBuiltinType() ||
      !llvm::cast<BuiltinType>(Idx->getType())->isInteger()) {
    emitError(Arr, "expression of type '" + Idx->getType()->toString() +
                       "' cannot be used as index in array");
  }
  ArrayType *ArrTy = llvm::cast<ArrayType>(Arr->getType());
  Expr->setType(ArrTy->getElemTy());
}

/* Insert declaration into the current scope */
void Sema::actOnVariableDecl(VariableDecl *Decl) {
  insertDecl(Decl);
  Decl->getInitializer()->sema(*this);
}

void Sema::actOnFunctionFragment(FunctionFragment *Fragment) {
  if (Declaration *PrevDecl = CurScope->find(Fragment->getId())) {
    /* Function already defined on some domain, add another domain */
    if (FunctionDecl *Decl = llvm::dyn_cast<FunctionDecl>(PrevDecl)) {
      Decl->appendFragment(Fragment->clone());
      return;
    }
    emitError(Fragment, "redefinition of '" + Fragment->getId() + "'");
    // TODO: print location of previous definition and its type
  }
  /* First definition of the function, create new FunctionDecl */
  auto Decl = std::make_unique<FunctionDecl>(Fragment->clone());
  CurScope->insert(Decl.get());
  FunctionDecls.push_back(std::move(Decl));
}

void Sema::actOnFuncParamDecl(FuncParamDecl *Decl) { insertDecl(Decl); }

/* Lookup declaration and set type of identifier */
void Sema::actOnIdentifier(Identifier *Id) {
  Declaration *Decl = findDecl(Id->getName());
  if (!Decl)
    emitError(Id, "use of undeclared identifier '" + Id->getName() + "'");
  Id->setDeclaration(Decl);
  Id->setType(Decl->getType());
}

void Sema::actOnReturnStmt(ReturnStmt *Ret) { Ret->getRetExpr()->sema(*this); }

void Sema::createEntryPoint() {
  auto It = llvm::find_if(FunctionDecls, [this](auto &&FD) {
    return FD->getId() == "main" && CurScope->contains(FD.get());
  });
  if (It == FunctionDecls.end())
    emitError("no main() function");
  // TODO: validate main() signature
  EntryPoint = getOrCreateFunctionInstance(It->get(), {});
}

void Sema::actOnTranslationUnit(TranslationUnit *TU) {
  llvm::for_each(TU->getNodes(), [this](auto &&N) { N->sema(*this); });
}

void Sema::run(TranslationUnit *TU) {
  Scope GlobalScope;
  CurScope = &GlobalScope;
  TU->sema(*this);
  createEntryPoint();
  assert(CurScope && !CurScope->getPrev() && "bug in scope handling");
}
