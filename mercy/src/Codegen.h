#pragma once
#include "AST.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>

namespace mercy {

class Codegen {
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> M;

  llvm::FunctionCallee getOrInsertPrintFunc(llvm::Type *ValueTy,
                                            const char *Postfix,
                                            const char *Fmt);
  llvm::FunctionCallee getOrInsertPrintFunc(BuiltinType *Ty);

public:
  Codegen();
  llvm::Value *emitIntegralLiteral(IntegralLiteral *IL);
  llvm::Value *emitBinaryOperator(BinaryOperator *BinOp);
  llvm::Value *emitUnaryOperator(UnaryOperator *Op);
  llvm::Value *emitFunctionCall(FunctionCall *FC);

  void run(std::unique_ptr<ASTNode> AST);

  /* Note that Module is binded to LLVMContext, i.e. Codegen must be alive
   * while returned module is used */
  std::unique_ptr<llvm::Module> takeResult() { return std::move(M); }
};

} // namespace mercy
