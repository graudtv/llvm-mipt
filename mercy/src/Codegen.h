#pragma once
#include "AST.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>

namespace mercy {

class Sema;

class Codegen {
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> M;

  llvm::FunctionCallee getOrInsertPrintFunc(llvm::Type *ValueTy,
                                            const char *Postfix,
                                            const char *Fmt);
  llvm::FunctionCallee getOrInsertPrintFunc(BuiltinType *Ty);

  llvm::FunctionCallee getOrInsertFunction(CallableFunction *F);
  llvm::FunctionCallee getOrInsertInstanceDecl(TemplateInstance *Instance);
  void emitTemplateInstance(TemplateInstance *Instance);

public:
  Codegen();
  llvm::Value *emitIntegralLiteral(IntegralLiteral *IL);
  llvm::Value *emitBinaryOperator(BinaryOperator *BinOp);
  llvm::Value *emitUnaryOperator(UnaryOperator *Op);
  llvm::Value *emitFunctionCall(FunctionCall *FC);
  llvm::Value *emitArraySubscriptExpr(ArraySubscriptExpr *Expr);
  llvm::Value *emitVariableDecl(VariableDecl *Decl);
  llvm::Value *emitFuncParamDecl(FuncParamDecl *Decl);
  llvm::Value *emitIdentifier(Identifier *Id);
  llvm::Value *emitReturnStmt(ReturnStmt *Ret);

  void run(TranslationUnit *TU, Sema &S);

  /* Note that Module is binded to LLVMContext, i.e. Codegen must be alive
   * while returned module is used */
  std::unique_ptr<llvm::Module> takeResult() { return std::move(M); }
};

} // namespace mercy
