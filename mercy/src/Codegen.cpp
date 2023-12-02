#include "AST.h"
#include "Codegen.h"
#include <llvm/Support/ErrorHandling.h>

using namespace mercy;

#define VISIT_NODE(NodeName)                                                   \
  llvm::Value *NodeName::codegen(Codegen &Gen) {                               \
    return Gen.emit##NodeName(this);                                           \
  }

#define UNREACHABLE_NODE(NodeName)                                             \
  llvm::Value *NodeName::codegen(Codegen &Gen) {                               \
    llvm_unreachable("unreachable node reached");                              \
  }

VISIT_NODE(IntegralLiteral)
VISIT_NODE(BinaryOperator)
VISIT_NODE(UnaryOperator)
VISIT_NODE(FunctionCall)

UNREACHABLE_NODE(ExpressionList)
UNREACHABLE_NODE(Identifier)

namespace {

void fatalError(const llvm::Twine &T) {
  llvm::errs() << T << '\n';
  exit(1);
}

} // 

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
    Bld.CreateCall(Printf,
                       {Bld.CreateGlobalStringPtr(Fmt, FuncName + "_fmt"),
                        Print->getArg(0)});
    Bld.CreateRetVoid();
  }
  return Callee;
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
  if (BinOp->getKind() == BinaryOperator::ADD)
    return Builder.CreateAdd(LHS, RHS);
  if (BinOp->getKind() == BinaryOperator::SUB)
    return Builder.CreateSub(LHS, RHS);
  if (BinOp->getKind() == BinaryOperator::MUL)
    return Builder.CreateMul(LHS, RHS);
  // TODO: handle uint types
  if (BinOp->getKind() == BinaryOperator::DIV)
    return Builder.CreateSDiv(LHS, RHS);
  if (BinOp->getKind() == BinaryOperator::REM)
    return Builder.CreateSRem(LHS, RHS);
  llvm_unreachable("unhandled binary operator kind");
}

llvm::Value *Codegen::emitUnaryOperator(UnaryOperator *Op) {
  llvm::Value *Expr = Op->getExpr()->codegen(*this);
  // TODO: emit errors for semantically invalid negations
  if (Op->getKind() == UnaryOperator::NEG)
    return Builder.CreateNeg(Expr);
  llvm_unreachable("unhandled unary operator kind");
}

llvm::Value *Codegen::emitFunctionCall(FunctionCall *FC) {
  ASTNode *Callee = FC->getCallee();

  /* Generate each function argument */
  std::vector<llvm::Value *> Arguments;
  for (size_t I = 0; I < FC->getArgCount(); ++I)
    Arguments.push_back(FC->getArg(I)->codegen(*this));

  assert(llvm::isa<Identifier>(Callee) && "indirect calls not implemented");
  // TODO: Id may be a function reference, not a function itself
  auto &Id = llvm::cast<Identifier>(Callee)->getName();
  if (Id == "print") {
    if (FC->getArgCount() != 1)
      fatalError("Invalid number of arguments in print() call");
    // TODO: call relevant print based on type
    llvm::FunctionCallee Print = getOrInsertPrintFunc(Builder.getInt32Ty(), "i32", "%d\n");
    return Builder.CreateCall(Print, Arguments);
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
  auto Last = AST->codegen(*this);
  Builder.CreateRet(Builder.getInt32(0));
}
