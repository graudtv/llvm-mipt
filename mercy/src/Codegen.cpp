#include "AST.h"
#include "Codegen.h"
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

SKIP_NODE(BuiltinTypeExpr)

UNREACHABLE_NODE(NodeList)
UNREACHABLE_NODE(Identifier)

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
  if (Ty->isInt8() || Ty->isInt16() || Ty->isInt32())
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

Codegen::Codegen() : Ctx(), Builder(Ctx) {
  M = std::make_unique<llvm::Module>("top", Ctx);
}

llvm::Value *Codegen::emitIntegralLiteral(IntegralLiteral *IL) {
  return Builder.getInt32(IL->getValue());
}

llvm::Value *Codegen::emitBinaryOperator(BinaryOperator *BinOp) {
  llvm::Value *LHSVal = BinOp->getLHS()->codegen(*this);
  llvm::Value *RHSVal = BinOp->getRHS()->codegen(*this);

  /* A little bit of semantic analys */
  Expression *LHS = llvm::cast<Expression>(BinOp->getLHS());
  Expression *RHS = llvm::cast<Expression>(BinOp->getRHS());
  if (!LHS->getType()->isBuiltinType())
    emitError(LHS, "invalid operand in binary operator");
  if (!RHS->getType()->isBuiltinType())
    emitError(RHS, "invalid operand in binary operator");
  if (LHS->getType() != RHS->getType())
    emitError(LHS, "operands of binary operator have different types");
  BuiltinType *OperandType = llvm::cast<BuiltinType>(LHS->getType());
  BinaryOperator::BinOpKind BK = BinOp->getKind();
  if (!OperandType->isInteger())
    emitError(BinOp, "invalid operand in binary operator, must be integer");
  BinOp->setType(OperandType);

  /* Codegen */
  if (BK == BinaryOperator::ADD)
    return Builder.CreateAdd(LHSVal, RHSVal);
  if (BK == BinaryOperator::SUB)
    return Builder.CreateSub(LHSVal, RHSVal);
  if (BK == BinaryOperator::MUL)
    return Builder.CreateMul(LHSVal, RHSVal);
  if (BK == BinaryOperator::DIV)
    return (OperandType->isSigned()) ? Builder.CreateSDiv(LHSVal, RHSVal)
                                     : Builder.CreateUDiv(LHSVal, RHSVal);
  if (BK == BinaryOperator::REM)
    return (OperandType->isSigned()) ? Builder.CreateSRem(LHSVal, RHSVal)
                                     : Builder.CreateURem(LHSVal, RHSVal);
  llvm_unreachable("unhandled binary operator kind");
}

llvm::Value *Codegen::emitUnaryOperator(UnaryOperator *Op) {
  llvm::Value *ExprVal = Op->getExpr()->codegen(*this);
  Expression *Expr = llvm::cast<Expression>(Op->getExpr());

  if (Op->getKind() == UnaryOperator::NEG) {
    if (Expr->getType() != BuiltinType::getBoolTy())
      emitError(Op, "invalid operand in negation, must be boolean");
    Op->setType(Expr->getType());
    return Builder.CreateNeg(ExprVal);
  }
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
      if (FC->getArgCount() != 1)
        emitError(Callee, "invalid number of arguments in type cast");
      auto *ArgTy = FC->getArg(0)->getType();
      if (!ArgTy->isBuiltinType() ||
          !llvm::cast<BuiltinType>(ArgTy)->isInteger() || !CastTy->isInteger())
        emitError(Callee, "cast from '" + ArgTy->toString() + "' to '" +
                              CastTy->toString() + "' is not allowed");

      FC->setType(CastTy);
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
      emitError(Callee, "invalid number of arguments in print() call");
    Expression *Arg = FC->getArg(0);
    llvm::FunctionCallee PrintFunc;
    if (auto *BuiltinTy = llvm::dyn_cast<BuiltinType>(Arg->getType())) {
      if (BuiltinTy->isVoid())
        emitError(Callee, "cannot print void");
      FC->setType(BuiltinType::getVoidTy());
      /* cast small integers to int32 / uint32 */
      if (BuiltinTy->isInt8() || BuiltinTy->isInt16() || BuiltinTy->isUint8() ||
          BuiltinTy->isUint16()) {
        Arguments.front() = Builder.CreateIntCast(
            Arguments.front(), Builder.getInt32Ty(), BuiltinTy->isSigned());
      }
      return Builder.CreateCall(getOrInsertPrintFunc(BuiltinTy), Arguments);
    } else {
      emitError(Callee, "cannot print argument of non-builtin type");
    }
  }
  assert(0 && "not implemented");
}

void Codegen::run(std::unique_ptr<ASTNode> AST) {
  M = std::make_unique<llvm::Module>("top", Ctx);

  llvm::FunctionType *MainFuncTy =
      llvm::FunctionType::get(Builder.getInt32Ty(), false);
  llvm::Function *MainFunc = llvm::Function::Create(
      MainFuncTy, llvm::Function::ExternalLinkage, "main", *M);
  llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(Ctx, "entry", MainFunc);
  Builder.SetInsertPoint(EntryBB);
  for (ASTNode *N : llvm::cast<NodeList>(AST.get())->getNodes())
    N->codegen(*this);
  Builder.CreateRet(Builder.getInt32(0));
}
