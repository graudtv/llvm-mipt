#pragma once

#include "AST.h"
#include <llvm/ADT/ArrayRef.h>
#include <unordered_map>

namespace mercy {

class Sema {
  using Scope = std::unordered_map<std::string, Declaration *>;
  using InstanceList = std::vector<std::unique_ptr<TemplateInstance>>;

  std::vector<Scope> Scopes;
  std::unordered_map<FunctionDecl *, InstanceList> FunctionInstances;

  /* Supplementary list which keeps track of all instantiations */
  std::vector<TemplateInstance *> AllInstances;

  Declaration *findDecl(const std::string &Id);
  void insertDecl(Declaration *Decl);
  void pushScope() { Scopes.emplace_back(); }
  void popScope() { Scopes.pop_back(); }

  TemplateInstance *
  getOrCreateFunctionInstance(FunctionDecl *FD,
                              llvm::ArrayRef<Type *> ParamTys);

public:
  void actOnBinaryOperator(BinaryOperator *BinOp);
  void actOnUnaryOperator(UnaryOperator *Op);
  void actOnFunctionCall(FunctionCall *FC);
  void actOnVariableDecl(VariableDecl *Decl);
  void actOnFunctionDecl(FunctionDecl *Decl);
  void actOnFuncParamDecl(FuncParamDecl *Decl);
  void actOnIdentifier(Identifier *Id);

  void run(ASTNode *TU);

  const auto &getGlobalFunctions() { return AllInstances; }
};

} // namespace mercy
