#include "AST.h"
#include "Codegen.h"
#include "Sema.h"
#include <llvm/Support/ErrorHandling.h>

using namespace mercy;

#define VISIT_NODE(NodeName)                                                   \
  llvm::Value *NodeName::codegen(Codegen &Gen) {                               \
    return Gen.emit##NodeName(this);                                           \
  }

#define SKIP_NODE(NodeName)                                                    \
  llvm::Value *NodeName::codegen(Codegen &Gen) { return nullptr; }

#define UNREACHABLE_NODE(NodeName)                                             \
  llvm::Value *NodeName::codegen(Codegen &Gen) {                               \
    llvm_unreachable("unreachable node reached");                              \
  }

VISIT_NODE(IntegralLiteral)
VISIT_NODE(BinaryOperator)
VISIT_NODE(UnaryOperator)
VISIT_NODE(FunctionCall)
VISIT_NODE(VariableDecl)
VISIT_NODE(Identifier)
VISIT_NODE(ReturnStmt)

SKIP_NODE(BuiltinTypeExpr)
SKIP_NODE(FunctionFragment)

UNREACHABLE_NODE(NodeList)
UNREACHABLE_NODE(FuncParamDecl)
UNREACHABLE_NODE(FunctionDecl)
UNREACHABLE_NODE(FunctionDomain)


namespace {

void emitError(ASTNode *Node, const llvm::Twine &T) {
  llvm::errs() << "line " << Node->getLocation().LineNo << ": " << T << '\n';
  exit(1);
}

} // namespace

/* Get or insert 'void @__print_TYPE(TYPE %arg)' function for builtin TYPE.
 * Fmt specifies format string to be passed to printf() */
llvm::FunctionCallee Codegen::getOrInsertPrintFunc(llvm::Type *ValueTy,
                                                   const char *Postfix,
                                                   const char *Fmt) {
  std::string FuncName = std::string("__print_") + Postfix;
  llvm::IRBuilder<> Bld(Ctx);

  /* int32 @printf(i8 *, ...) */
  llvm::FunctionType *PrintfType = llvm::FunctionType::get(
      Bld.getInt32Ty(), Bld.getInt8Ty()->getPointerTo(), true);
  llvm::FunctionCallee Printf = M->getOrInsertFunction("printf", PrintfType);

  llvm::FunctionType *PrintType =
      llvm::FunctionType::get(Bld.getVoidTy(), ValueTy, false);
  llvm::FunctionCallee Callee = M->getOrInsertFunction(FuncName, PrintType);
  llvm::Function *Print = llvm::cast<llvm::Function>(Callee.getCallee());
  if (Print->empty()) {
    llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Ctx, "", Print);
    Bld.SetInsertPoint(EntryBB);
    Bld.CreateCall(Printf, {Bld.CreateGlobalStringPtr(Fmt, FuncName + "_fmt"),
                            Print->getArg(0)});
    Bld.CreateRetVoid();
  }
  return Callee;
}

llvm::FunctionCallee Codegen::getOrInsertPrintFunc(BuiltinType *Ty) {
  if (Ty->isBool() || Ty->isInt8() || Ty->isInt16() || Ty->isInt32())
    return getOrInsertPrintFunc(Builder.getInt32Ty(), "i32", "%d\n");
  if (Ty->isUint8() || Ty->isUint16() || Ty->isUint32())
    return getOrInsertPrintFunc(Builder.getInt32Ty(), "u32", "%u\n");
  if (Ty->isInt64())
    return getOrInsertPrintFunc(Builder.getInt64Ty(), "i64", "%ld\n");
  if (Ty->isUint64())
    return getOrInsertPrintFunc(Builder.getInt64Ty(), "u64", "%lu\n");
  if (Ty->isUintptr())
    return getOrInsertPrintFunc(Ty->getLLVMType(Ctx), "uintptr", "0x%lx\n");
  llvm_unreachable("unhandled builtin type");
}

llvm::FunctionCallee Codegen::getOrInsertFunction(CallableFunction *F) {
  if (auto *I = llvm::dyn_cast<TemplateInstance>(F))
    return getOrInsertInstanceDecl(I);
  assert(0 && "extern not implemented");
}

llvm::FunctionCallee Codegen::getOrInsertInstanceDecl(TemplateInstance *Instance) {
  if (llvm::Function *F = M->getFunction(Instance->getId()))
    return F;

  // TODO: Handle non-builtin return types
  FunctionDecl *FD = Instance->getDecl();
  assert(llvm::isa<BuiltinType>(FD->getReturnType()));
  BuiltinType *RetTy = llvm::cast<BuiltinType>(FD->getReturnType());

  // TODO: Handle non-builtin parameter types
  std::vector<llvm::Type *> ParamTys;
  llvm::transform(
      FD->getParamTypes(), std::back_inserter(ParamTys),
      [this](Type *T) { return llvm::cast<BuiltinType>(T)->getLLVMType(Ctx); });
  llvm::FunctionType *FuncTy =
      llvm::FunctionType::get(RetTy->getLLVMType(Ctx), ParamTys, false);
  return M->getOrInsertFunction(Instance->getId(), FuncTy);
}

Codegen::Codegen() : Ctx(), Builder(Ctx) {
  M = std::make_unique<llvm::Module>("top", Ctx);
}

llvm::Value *Codegen::emitIntegralLiteral(IntegralLiteral *IL) {
  return Builder.getInt32(IL->getValue());
}

llvm::Value *Codegen::emitBinaryOperator(BinaryOperator *BinOp) {
  llvm::Value *LHS = BinOp->getLHS()->codegen(*this);
  llvm::Value *RHS = BinOp->getRHS()->codegen(*this);

  BuiltinType *ResTy = llvm::cast<BuiltinType>(BinOp->getType());
  BuiltinType *OperandTy = llvm::cast<BuiltinType>(BinOp->getLHS()->getType());
  switch (BinOp->getKind()) {
  case BinaryOperator::LOR:
    return Builder.CreateLogicalOr(LHS, RHS);
  case BinaryOperator::LAND:
    return Builder.CreateLogicalAnd(LHS, RHS);
  case BinaryOperator::OR:
    return Builder.CreateOr(LHS, RHS);
  case BinaryOperator::XOR:
    return Builder.CreateXor(LHS, RHS);
  case BinaryOperator::AND:
    return Builder.CreateAnd(LHS, RHS);
  case BinaryOperator::EQ:
    return Builder.CreateICmpEQ(LHS, RHS);
  case BinaryOperator::NE:
    return Builder.CreateICmpNE(LHS, RHS);
  case BinaryOperator::LE:
    return (OperandTy->isSigned()) ? Builder.CreateICmpSLE(LHS, RHS)
                                   : Builder.CreateICmpULE(LHS, RHS);
  case BinaryOperator::GE:
    return (OperandTy->isSigned()) ? Builder.CreateICmpSGE(LHS, RHS)
                                   : Builder.CreateICmpUGE(LHS, RHS);
  case BinaryOperator::LT:
    return (OperandTy->isSigned()) ? Builder.CreateICmpSLT(LHS, RHS)
                                   : Builder.CreateICmpULT(LHS, RHS);
  case BinaryOperator::GT:
    return (OperandTy->isSigned()) ? Builder.CreateICmpSGT(LHS, RHS)
                                   : Builder.CreateICmpUGT(LHS, RHS);
  case BinaryOperator::LSHIFT:
    return Builder.CreateShl(LHS, RHS);
  case BinaryOperator::RSHIFT:
    return (ResTy->isSigned()) ? Builder.CreateAShr(LHS, RHS)
                               : Builder.CreateLShr(LHS, RHS);
  case BinaryOperator::ADD:
    return Builder.CreateAdd(LHS, RHS);
  case BinaryOperator::SUB:
    return Builder.CreateSub(LHS, RHS);
  case BinaryOperator::MUL:
    return Builder.CreateMul(LHS, RHS);
  case BinaryOperator::DIV:
    return (ResTy->isSigned()) ? Builder.CreateSDiv(LHS, RHS)
                               : Builder.CreateUDiv(LHS, RHS);
  case BinaryOperator::REM:
    return (ResTy->isSigned()) ? Builder.CreateSRem(LHS, RHS)
                               : Builder.CreateURem(LHS, RHS);
  }
  llvm_unreachable("unhandled binary operator kind");
}

llvm::Value *Codegen::emitUnaryOperator(UnaryOperator *Op) {
  llvm::Value *Expr = Op->getExpr()->codegen(*this);
  if (Op->getKind() == UnaryOperator::NOT)
    return Builder.CreateNot(Expr);
  llvm_unreachable("unhandled unary operator kind");
}

llvm::Value *Codegen::emitFunctionCall(FunctionCall *FC) {
  ASTNode *Callee = FC->getCallee();

  /* Generate each function argument */
  std::vector<llvm::Value *> Arguments;
  for (Expression *Arg : FC->getArgs())
    Arguments.push_back(Arg->codegen(*this));

  /* Handle type conversions */
  if (auto *BTE = llvm::dyn_cast<BuiltinTypeExpr>(Callee)) {
    /* Conversion to builtin type */
    if (auto *CastTy = llvm::dyn_cast<BuiltinType>(BTE->getReferencedType())) {
      return Builder.CreateIntCast(Arguments.front(), CastTy->getLLVMType(Ctx),
                                   CastTy->isSigned());
    }
    // TODO: array constructor
    assert(0 && "unimplemented or illegal cast");
  }
  assert(llvm::isa<Identifier>(Callee) && "indirect calls not implemented");
  // TODO: Id may be a function reference, not a function itself
  auto &Id = llvm::cast<Identifier>(Callee)->getName();
  if (Id == "print") {
    if (FC->getArgCount() != 1)
      emitError(FC, "invalid number of arguments in print() call");
    Expression *Arg = FC->getArg(0);
    llvm::FunctionCallee PrintFunc;
    if (auto *BuiltinTy = llvm::dyn_cast<BuiltinType>(Arg->getType())) {
      if (BuiltinTy->isVoid())
        emitError(Callee, "cannot print void");
      FC->setType(BuiltinType::getVoidTy());
      /* cast small integers to int32 / uint32 */
      if (BuiltinTy->isBool() || BuiltinTy->isInt8() || BuiltinTy->isInt16() ||
          BuiltinTy->isUint8() || BuiltinTy->isUint16()) {
        Arguments.front() = Builder.CreateIntCast(
            Arguments.front(), Builder.getInt32Ty(), BuiltinTy->isSigned());
      }
      return Builder.CreateCall(getOrInsertPrintFunc(BuiltinTy), Arguments);
    } else {
      emitError(Callee, "cannot print argument of non-builtin type");
    }
  }
  return Builder.CreateCall(getOrInsertFunction(FC->getCalleeFunc()), Arguments);
}

llvm::Value *Codegen::emitVariableDecl(VariableDecl *Decl) {
  assert(!Decl->isRef() && "references not implemented");
  assert(llvm::isa<BuiltinType>(Decl->getInitializer()->getType()) &&
         "only builtin type decls are implemented");

  BuiltinType *ExprTy =
      llvm::cast<BuiltinType>(Decl->getInitializer()->getType());
  Decl->setAddr(Builder.CreateAlloca(ExprTy->getLLVMType(Ctx)));
  llvm::Value *Initializer = Decl->getInitializer()->codegen(*this);
  Builder.CreateStore(Initializer, Decl->getAddr());
  return nullptr;
}

void Codegen::emitTemplateInstance(TemplateInstance *Instance) {
  FunctionDecl *FD = Instance->getDecl();
  FunctionFragment *FrontFragment = FD->getFragment(0);

  llvm::Function *Func =
      llvm::cast<llvm::Function>(getOrInsertInstanceDecl(Instance).getCallee());
  llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Ctx, "", Func);
  Builder.SetInsertPoint(EntryBB);

  /* Create alloca for each function parameter */
  for (size_t I = 0; I < FrontFragment->getParamCount(); ++I) {
    FuncParamDecl *Param = FrontFragment->getParam(I);
    Func->getArg(I)->setName(Param->getId());
    assert(llvm::isa<BuiltinType>(Param->getType()) &&
           "only builtin types implemented");
    llvm::Type *Ty =
        llvm::cast<BuiltinType>(Param->getType())->getLLVMType(Ctx);
    Param->setAddr(Builder.CreateAlloca(Ty, nullptr, Param->getId() + ".copy"));
    Builder.CreateStore(Func->getArg(I), Param->getAddr());
  }
  /* Handle each domain */
  for (FunctionFragment *Fragment : FD->getFragments()) {
    llvm::BasicBlock *NextBB = nullptr;
    /* Bind parameters to addresses */
    for (size_t I = 0; I < FrontFragment->getParamCount(); ++I)
      Fragment->getParam(I)->setAddr(FrontFragment->getParam(I)->getAddr());
    /* Emit WhenExpr if exists */
    if (Fragment->getDomain()->isCustom()) {
      llvm::Value *Cond = Fragment->getDomain()->getExpr()->codegen(*this);
      llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(Ctx, "domain", Func);
      NextBB = llvm::BasicBlock::Create(Ctx, "next", Func);
      Builder.CreateCondBr(Cond, TrueBB, NextBB);
      Builder.SetInsertPoint(TrueBB);
    }
    /* Generate function body */
    bool HasRetInstr = false;
    for (ASTNode *Stmt : Fragment->getBody()) {
      llvm::Value *V = Stmt->codegen(*this);
      if (llvm::isa<ReturnStmt>(Stmt)) {
        HasRetInstr = true;
        break;
      }
    }
    if (!HasRetInstr)
      Builder.CreateRetVoid();
    /* Move to the next domain */
    if (NextBB)
      Builder.SetInsertPoint(NextBB);
  }
  /* Runtime 'domain error' emission */
  FunctionFragment *LastFragment = *std::prev(FD->getFragments().end());
  if (LastFragment->getDomain()->isCustom()) {
    llvm::FunctionCallee Puts = M->getOrInsertFunction(
        "puts", Builder.getInt32Ty(), Builder.getInt8Ty()->getPointerTo());
    llvm::FunctionCallee Exit = M->getOrInsertFunction(
        "exit", Builder.getVoidTy(), Builder.getInt32Ty());
    Builder.CreateCall(Puts, Builder.CreateGlobalStringPtr(
                                 "Error: domain error in function '" +
                                 Instance->getId() + "'"));
    Builder.CreateCall(Exit, Builder.getInt32(1));
    assert(llvm::isa<BuiltinType>(FD->getReturnType()) &&
           "only builtin types implemented");
    BuiltinType *RetTy = llvm::cast<BuiltinType>(FD->getReturnType());
    Builder.CreateUnreachable();
  }
}

llvm::Value *Codegen::emitIdentifier(Identifier *Id) {
  assert(llvm::isa<BuiltinType>(Id->getType()) &&
         "only builtin types are implemented");
  BuiltinType *IdTy = llvm::cast<BuiltinType>(Id->getType());
  return Builder.CreateLoad(IdTy->getLLVMType(Ctx),
                            Id->getDeclaration()->getAddr());
}

llvm::Value *Codegen::emitReturnStmt(ReturnStmt *Ret) {
  llvm::Value *V = Ret->getRetExpr()->codegen(*this);
  if (Ret->getRetType() == BuiltinType::getVoidTy())
    return Builder.CreateRetVoid();
  return Builder.CreateRet(V);
}

void Codegen::run(ASTNode *AST, Sema &S) {
  M = std::make_unique<llvm::Module>("top", Ctx);

  for (TemplateInstance *Instance : S.getGlobalFunctions())
    emitTemplateInstance(Instance);

  llvm::FunctionType *MainFuncTy =
      llvm::FunctionType::get(Builder.getInt32Ty(), false);
  llvm::Function *MainFunc = llvm::Function::Create(
      MainFuncTy, llvm::Function::ExternalLinkage, "main", *M);
  llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Ctx, "entry", MainFunc);
  Builder.SetInsertPoint(EntryBB);
  auto Statements = llvm::cast<NodeList>(AST)->getNodes();
  for (auto S : Statements)
    S->codegen(*this);
  Builder.CreateRet(Builder.getInt32(0));
  for (auto S : Statements)
    if (llvm::isa<FunctionFragment>(S))
      S->codegen(*this);
}
