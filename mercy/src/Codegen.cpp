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
VISIT_NODE(StringLiteral)
VISIT_NODE(BinaryOperator)
VISIT_NODE(UnaryOperator)
VISIT_NODE(FunctionCall)
VISIT_NODE(ArraySubscriptExpr)
VISIT_NODE(VariableDecl)
VISIT_NODE(Identifier)
VISIT_NODE(ReturnStmt)

SKIP_NODE(TypeExpr)
SKIP_NODE(FunctionFragment)

UNREACHABLE_NODE(NodeList)
UNREACHABLE_NODE(FuncParamDecl)
UNREACHABLE_NODE(FunctionDecl)
UNREACHABLE_NODE(FunctionDomain)
UNREACHABLE_NODE(TranslationUnit)

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
  if (Ty->isString())
    return getOrInsertPrintFunc(Ty->getLLVMType(Ctx), "str", "%s\n");
  llvm_unreachable("unhandled builtin type");
}

llvm::FunctionCallee Codegen::getOrInsertFunction(CallableFunction *F) {
  if (auto *I = llvm::dyn_cast<TemplateInstance>(F))
    return getOrInsertInstanceDecl(I);
  assert(0 && "extern not implemented");
}

llvm::FunctionCallee
Codegen::getOrInsertInstanceDecl(TemplateInstance *Instance) {
  if (llvm::Function *F = M->getFunction(Instance->getId()))
    return F;

  FunctionDecl *FD = Instance->getDecl();

  std::vector<llvm::Type *> ParamTys;
  llvm::transform(FD->getParamTypes(), std::back_inserter(ParamTys),
                  [this](Type *T) { return T->getLLVMType(Ctx); });
  llvm::FunctionType *FuncTy = llvm::FunctionType::get(
      FD->getReturnType()->getLLVMType(Ctx), ParamTys, false);
  return M->getOrInsertFunction(Instance->getId(), FuncTy);
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
    llvm::Type *Ty = Param->getType()->getLLVMType(Ctx);
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
      llvm::Value *Cond = emitAsRValue(Fragment->getDomain()->getExpr());
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

/* When emitting lvalues, their address is returned instead of the value
 * itself, because lvalues can also occur as lhs in assignment. This function
 * performs lvalue to rvalue propogation if necessary */
llvm::Value *Codegen::emitAsRValue(Expression *E) {
  llvm::Value *V = E->codegen(*this);
  if (E->isLValue())
    return Builder.CreateLoad(E->getType()->getLLVMType(Ctx), V);
  return V;
}

Codegen::Codegen() : Ctx(), Builder(Ctx) {
  M = std::make_unique<llvm::Module>("top", Ctx);
}

llvm::Value *Codegen::emitIntegralLiteral(IntegralLiteral *IL) {
  return Builder.getInt32(IL->getValue());
}

llvm::Value *Codegen::emitStringLiteral(StringLiteral *Str) {
  return Builder.CreateGlobalStringPtr(Str->getValue());
}

llvm::Value *Codegen::emitBinaryOperator(BinaryOperator *BinOp) {
  bool IsAssignment = BinOp->getKind() == BinaryOperator::ASSIGN;
  llvm::Value *LHS = IsAssignment ? BinOp->getLHS()->codegen(*this)
                                  : emitAsRValue(BinOp->getLHS());
  llvm::Value *RHS = emitAsRValue(BinOp->getRHS());

  BuiltinType *ResTy = llvm::cast<BuiltinType>(BinOp->getType());
  BuiltinType *OperandTy = llvm::cast<BuiltinType>(BinOp->getLHS()->getType());
  switch (BinOp->getKind()) {
  case BinaryOperator::ASSIGN:
    Builder.CreateStore(RHS, LHS);
    return RHS;
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
  llvm::Value *Expr = emitAsRValue(Op->getExpr());
  if (Op->getKind() == UnaryOperator::NOT)
    return Builder.CreateNot(Expr);
  llvm_unreachable("unhandled unary operator kind");
}

llvm::Value *Codegen::emitFunctionCall(FunctionCall *FC) {
  Expression *Callee = FC->getCallee();

  /* Generate each function argument */
  std::vector<llvm::Value *> Arguments;
  for (Expression *Arg : FC->getArgs())
    Arguments.push_back(emitAsRValue(Arg));

  /* Handle type conversions */
  if (auto *TE = llvm::dyn_cast<TypeExpr>(Callee)) {
    /* Conversion to builtin type */
    if (auto *CastTy = llvm::dyn_cast<BuiltinType>(TE->getValue())) {
      return Builder.CreateIntCast(Arguments.front(), CastTy->getLLVMType(Ctx),
                                   CastTy->isSigned());
    }
    assert(0 && "unimplemented or illegal cast");
  }
  // TODO: Id may be a function reference, not a function itself
  if (Identifier *Id = llvm::dyn_cast<Identifier>(Callee)) {
    if (Id->getName() == "array") {
      assert(llvm::isa<ArrayType>(FC->getType()));
      Type *ElemTy = llvm::cast<ArrayType>(FC->getType())->getElemTy();
      assert(llvm::isa<BuiltinType>(ElemTy) &&
             "arrays of non-builtin types are not implemented");
      llvm::Type *LLVMElemTy =
          llvm::cast<BuiltinType>(ElemTy)->getLLVMType(Ctx);
      llvm::Value *Arr = Builder.CreateAlloca(
          LLVMElemTy, Builder.getInt32(Arguments.size()), "anon_array");

      for (size_t I = 0; I < Arguments.size(); ++I) {
        llvm::Value *Ptr = Builder.CreateConstInBoundsGEP1_32(
            LLVMElemTy, Arr, I, "anon_array_elem_ptr");
        Builder.CreateStore(Arguments[I], Ptr);
      }
      return Arr;
    }
    if (Id->getName() == "alloca") {
      assert(llvm::isa<ArrayType>(FC->getType()));
      Type *ElemTy = llvm::cast<ArrayType>(FC->getType())->getElemTy();
      return Builder.CreateAlloca(ElemTy->getLLVMType(Ctx), Arguments[1],
                                  "anon_array");
    }
    if (Id->getName() == "extern") {
      auto &SymbolName = FC->getArg(0)->getConstexprValue().asString();
      llvm::Type *SymbolType = FC->getType()->getLLVMType(Ctx);
      /* Extern function */
      if (FC->isConstexpr()) {
        M->getOrInsertFunction(SymbolName,
                               llvm::cast<llvm::FunctionType>(SymbolType));
        return nullptr;
      }
      /* Extern variable */
      llvm::Value *Addr = M->getOrInsertGlobal(SymbolName, SymbolType);
      return Builder.CreateLoad(SymbolType, Addr, SymbolName);
    }
    if (Id->getName() == "function_type") {
      /* do nothing, compile-time evaluated by Sema */
      return nullptr;
    }
    if (Id->getName() == "print") {
      assert(FC->getArgCount() == 1 &&
             "invalid number of arguments in print()");
      Expression *Arg = FC->getArg(0);
      llvm::FunctionCallee PrintFunc;
      if (auto *BuiltinTy = llvm::dyn_cast<BuiltinType>(Arg->getType())) {
        /* cast small integers to int32 / uint32 */
        if (BuiltinTy->isBool() || BuiltinTy->isInt8() ||
            BuiltinTy->isInt16() || BuiltinTy->isUint8() ||
            BuiltinTy->isUint16()) {
          Arguments.front() = Builder.CreateIntCast(
              Arguments.front(), Builder.getInt32Ty(), BuiltinTy->isSigned());
        }
        return Builder.CreateCall(getOrInsertPrintFunc(BuiltinTy), Arguments);
      } else {
        emitError(Callee, "cannot print argument of non-builtin type");
      }
    }
    return Builder.CreateCall(getOrInsertFunction(FC->getCalleeFunc()),
                              Arguments);
  }
  assert(Callee->isConstexpr() && "unhandled constexpr in Sema");
  if (Callee->getConstexprValue().isExternFuncRef()) {
    auto *F = Callee->getConstexprValue().asExternFuncRef();
    llvm::FunctionCallee Callee = M->getOrInsertFunction(
        F->getId(),
        llvm::cast<llvm::FunctionType>(F->getType()->getLLVMType(Ctx)));
    return Builder.CreateCall(Callee, Arguments);
  }
  llvm_unreachable("unhandled function call");
}

llvm::Value *Codegen::emitArraySubscriptExpr(ArraySubscriptExpr *Expr) {
  llvm::Value *Arr = emitAsRValue(Expr->getArray());
  llvm::Value *Idx = emitAsRValue(Expr->getIndex());

  assert(llvm::isa<BuiltinType>(Expr->getType()) &&
         "arrays of non-builtin types are not implemented");
  llvm::Type *ElemTy =
      llvm::cast<BuiltinType>(Expr->getType())->getLLVMType(Ctx);

  return Builder.CreateGEP(ElemTy, Arr, Idx, "elem_ptr");
}

llvm::Value *Codegen::emitVariableDecl(VariableDecl *Decl) {
  assert(!Decl->isRef() && "references not implemented");
  llvm::Value *Initializer = emitAsRValue(Decl->getInitializer());
  llvm::Value *Addr =
      Builder.CreateAlloca(Initializer->getType(), nullptr, Decl->getId());
  Decl->setAddr(Addr);
  Builder.CreateStore(Initializer, Addr);
  return nullptr;
}

llvm::Value *Codegen::emitIdentifier(Identifier *Id) {
  return Id->getDeclaration()->getAddr();
}

llvm::Value *Codegen::emitReturnStmt(ReturnStmt *Ret) {
  llvm::Value *V = emitAsRValue(Ret->getRetExpr());
  if (Ret->getRetType() == BuiltinType::getVoidTy())
    return Builder.CreateRetVoid();
  return Builder.CreateRet(V);
}

void Codegen::run(TranslationUnit *TU, Sema &S) {
  M = std::make_unique<llvm::Module>("top", Ctx);

  /* Emit global variables */
  for (ASTNode *N : TU->getNodes())
    if (VariableDecl *Decl = llvm::dyn_cast<VariableDecl>(N)) {
      M->getOrInsertGlobal(Decl->getId(), Decl->getType()->getLLVMType(Ctx));
      llvm::GlobalVariable *V = M->getNamedGlobal(Decl->getId());
      V->setInitializer(
          llvm::PoisonValue::get(Decl->getType()->getLLVMType(Ctx)));
      V->setLinkage(llvm::GlobalValue::PrivateLinkage);
      Decl->setAddr(V);
    }

  /* Emit all functions */
  for (TemplateInstance *Instance : S.getGlobalFunctions())
    emitTemplateInstance(Instance);

  /* Create main() */
  llvm::FunctionType *MainFuncTy =
      llvm::FunctionType::get(Builder.getInt32Ty(), false);
  llvm::Function *MainFunc = llvm::Function::Create(
      MainFuncTy, llvm::Function::ExternalLinkage, "main", *M);
  llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Ctx, "", MainFunc);
  Builder.SetInsertPoint(EntryBB);
  llvm::FunctionCallee EntryFunc = M->getFunction(S.getEntryPoint()->getId());
  assert(EntryFunc && "no entry point");

  /* Init global variables */
  for (ASTNode *N : TU->getNodes())
    if (VariableDecl *Decl = llvm::dyn_cast<VariableDecl>(N)) {
      llvm::GlobalVariable *V = M->getNamedGlobal(Decl->getId());
      Builder.CreateStore(emitAsRValue(Decl->getInitializer()), V);
    }

  Builder.CreateCall(EntryFunc);
  Builder.CreateRet(Builder.getInt32(0));
}
