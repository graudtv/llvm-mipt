#pragma once

#include "AST.h"
#include <llvm/ADT/ArrayRef.h>
#include <unordered_map>
#include <list>

namespace mercy {

class Scope {
  Scope *Prev = nullptr;
  std::string Name;
  std::unordered_map<std::string, Declaration *> Decls;

public:
  Scope(Scope *PrevScope = nullptr, std::string ScopeName = {})
      : Prev(PrevScope), Name(std::move(ScopeName)) {}

  bool isGlobal() const { return !Prev; }
  Scope *getPrev() { return Prev; }
  const std::string &getName() { return Name; }

  bool contains(Declaration *Decl) {
    auto It = llvm::find_if(
        Decls, [Decl](auto &&Pair) { return Pair.second == Decl; });
    return It != Decls.end();
  }

  Declaration *find(const std::string &Id) {
    if (auto It = Decls.find(Id); It != Decls.end())
      return It->second;
    return nullptr;
  }

  void insert(Declaration *Decl) {
    Decls.insert(std::make_pair(Decl->getId(), Decl));
  }
};

class Sema {
  Scope *CurScope = nullptr;
  using InstanceList = std::vector<TemplateInstance *>;
  std::unordered_map<FunctionDecl *, InstanceList> FunctionInstances;

  /* Storage for all emitted function declarations */
  std::vector<std::unique_ptr<FunctionDecl>> FunctionDecls;
  /* Storage for all emitted function instantiations */
  std::vector<TemplateInstance *> AllInstances;
  /* Instance of main() */
  TemplateInstance *EntryPoint = nullptr;
  /* List of used extern functions */
  std::list<ExternFunction> ExternFunctions;

  Declaration *findDecl(const std::string &Id);
  void insertDecl(Declaration *Decl);

  TemplateInstance *
  getOrCreateFunctionInstance(FunctionDecl *FD,
                              llvm::ArrayRef<Type *> ParamTys);

  void actOnFunctionBody(FunctionFragment *Func);
  void createEntryPoint();

public:
  void actOnBinaryOperator(BinaryOperator *BinOp);
  void actOnUnaryOperator(UnaryOperator *Op);
  void actOnFunctionCall(FunctionCall *FC);
  void actOnArraySubscriptExpr(ArraySubscriptExpr *Expr);
  void actOnVariableDecl(VariableDecl *Decl);
  void actOnFunctionFragment(FunctionFragment *Fragment);
  void actOnFuncParamDecl(FuncParamDecl *Decl);
  void actOnIdentifier(Identifier *Id);
  void actOnReturnStmt(ReturnStmt *Ret);
  void actOnTranslationUnit(TranslationUnit *TU);

  void run(TranslationUnit *TU);

  const auto &getGlobalFunctions() { return AllInstances; }
  TemplateInstance *getEntryPoint() { return EntryPoint; }
};

} // namespace mercy
