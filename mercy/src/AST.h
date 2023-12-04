#pragma once

#include "Type.h"
#include "misc.h"
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

namespace llvm {
class Value;
class Function;
} // namespace llvm

namespace mercy {

class Codegen;
class Sema;
class Declaration;
class CallableFunction;

struct Location {
  unsigned LineNo = 0;

  bool isValid() const { return LineNo != 0; }
};

class ASTNode : private NonCopyable {
public:
  enum NodeKind {
    NK_IntegralLiteral,
    NK_BinaryOperator,
    NK_UnaryOperator,
    NK_Identifier,
    NK_FunctionCall,
    NK_NodeList,
    NK_VariableDecl,
    NK_FuncParamDecl,
    NK_FunctionDecl,
    NK_BuiltinTypeExpr,

    NK_CallableFunction,
    NK_TemplateInstance,
    NK_ExternFunction
  };

private:
  NodeKind NK;
  Location Loc;

public:
  ASTNode(NodeKind K) : NK(K) {}
  virtual ~ASTNode() {}

  NodeKind getNodeKind() const { return NK; }
  virtual void print(llvm::raw_ostream &Os, unsigned Shift) const = 0;
  virtual llvm::Value *codegen(Codegen &Gen) = 0;
  virtual void sema(Sema &S) = 0;
  virtual ASTNode *clone() const = 0;

  void setLocation(Location L) { Loc = L; }
  Location getLocation() const { return Loc; }

  void print(llvm::raw_ostream &Os = llvm::outs()) const { print(Os, 0); }
};

/* Base class for all nodes that have a type.
 * For most expressions type is initialized during semantic analysis */
class Expression : public ASTNode {
  Type *Ty;

public:
  Type *getType() const { return Ty; }
  void setType(Type *T) { Ty = T; }
  virtual Expression *clone() const = 0;

  Expression(NodeKind NK, Type *T = nullptr) : ASTNode(NK), Ty(T) {}

  static bool classof(const ASTNode *N) {
    NodeKind NK = N->getNodeKind();
    return NK == NK_IntegralLiteral || NK == NK_BinaryOperator ||
           NK == NK_UnaryOperator || NK == NK_Identifier ||
           NK == NK_FunctionCall || NK == NK_BuiltinTypeExpr;
  }
};

class IntegralLiteral : public Expression {
  int Value;

public:
  IntegralLiteral(int V)
      : Expression(NK_IntegralLiteral, BuiltinType::getIntTy()), Value(V) {}

  int getValue() const { return Value; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  IntegralLiteral *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_IntegralLiteral;
  }
};

class BinaryOperator : public Expression {
public:
  // clang-format off
  enum BinOpKind {
    LOR, LAND, OR, XOR, AND,
    EQ, NE, LT, GT, LE, GE, LSHIFT, RSHIFT,
    ADD, SUB, MUL, DIV, REM
  };
  // clang-format on

private:
  BinOpKind Kind;
  std::unique_ptr<Expression> LHS;
  std::unique_ptr<Expression> RHS;

public:
  BinaryOperator(BinOpKind K, Expression *L, Expression *R)
      : Expression(NK_BinaryOperator), Kind(K), LHS(L), RHS(R) {}

  Expression *getLHS() { return LHS.get(); }
  Expression *getRHS() { return RHS.get(); }
  BinOpKind getKind() const { return Kind; }

  const char *getMnemonic() const;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  BinaryOperator *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_BinaryOperator;
  }
};

class UnaryOperator : public Expression {
public:
  enum UnaryOpKind { NOT };

private:
  UnaryOpKind Kind;
  std::unique_ptr<Expression> Expr;

public:
  UnaryOperator(UnaryOpKind K, Expression *E)
      : Expression(NK_UnaryOperator), Kind(K), Expr(E) {}

  Expression *getExpr() { return Expr.get(); }
  UnaryOpKind getKind() const { return Kind; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  UnaryOperator *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_UnaryOperator;
  }
};

/* Identifier referencing to some object */
class Identifier : public Expression {
  std::string Name;
  Declaration *Decl = nullptr;

public:
  Identifier(std::string Id) : Expression(NK_Identifier), Name(std::move(Id)) {}
  const std::string &getName() const { return Name; }

  /* Set during Sema */
  void setDeclaration(Declaration *D) { Decl = D; }
  Declaration *getDeclaration() { return Decl; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  Identifier *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_Identifier;
  }
};

class NodeList : public ASTNode {
  std::vector<std::unique_ptr<ASTNode>> Nodes;

public:
  NodeList() : ASTNode(NK_NodeList) {}
  NodeList(ASTNode *N) : NodeList() { append(N); }

  void append(ASTNode *N) { Nodes.emplace_back(N); }

  size_t size() const { return Nodes.size(); }
  ASTNode *getNode(size_t Idx) { return Nodes[Idx].get(); }

  template <class NodeT> NodeT *getNodeAs(size_t Idx) {
    return llvm::cast<NodeT>(Nodes[Idx].get());
  }

  template <class NodeT> auto getNodesAs() {
    return llvm::map_range(Nodes,
                           [](auto &&N) { return llvm::cast<NodeT>(N.get()); });
  }
  auto getNodes() { return getNodesAs<ASTNode>(); }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  NodeList *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_NodeList;
  }
};

class Declaration : public ASTNode {
  std::string Id;
  std::unique_ptr<Expression> Init;
  llvm::Value *Addr = nullptr;

public:
  Declaration(NodeKind NK, std::string Identifier, Expression *Initializer)
      : ASTNode(NK), Id(std::move(Identifier)), Init(Initializer) {}

  const std::string &getId() const { return Id; }

  Expression *getInitializer() { return Init.get(); }
  const Expression *getInitializer() const { return Init.get(); }

  /* May be used by Codegen */
  void setAddr(llvm::Value *A) { Addr = A; }
  llvm::Value *getAddr() { return Addr; }

  virtual Type *getType() = 0;

  static bool classof(const ASTNode *N) {
    NodeKind NK = N->getNodeKind();
    return NK == NK_VariableDecl || NK == NK_FuncParamDecl ||
           NK == NK_FunctionDecl;
  }
};

/* Variable or type alias declaration */
class VariableDecl : public Declaration {
  bool IsRef;

public:
  VariableDecl(std::string Id, Expression *E, bool IsReference)
      : Declaration(NK_VariableDecl, std::move(Id), E), IsRef(IsReference) {}

  bool isRef() const { return IsRef; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  VariableDecl *clone() const override;
  Type *getType() override { return getInitializer()->getType(); }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_VariableDecl;
  }
};

class FuncParamDecl : public Declaration {
  bool IsRef;
  Type *ParamTy = nullptr;
  llvm::Value *Addr = nullptr;

public:
  FuncParamDecl(std::string Id, bool IsReference)
      : Declaration(NK_FuncParamDecl, std::move(Id), nullptr),
        IsRef(IsReference) {}

  bool isRef() const { return IsRef; }

  /* Set by Sema when instantiating functions */
  void setType(Type *T) { ParamTy = T; }
  Type *getType() override { return ParamTy; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  FuncParamDecl *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_FuncParamDecl;
  }
};

class FunctionDecl : public Declaration {
  std::unique_ptr<NodeList> Params;
  std::unique_ptr<Expression> WhenExpr;

public:
  FunctionDecl(std::string Id, NodeList *ParamList, Expression *Init,
               Expression *When = nullptr)
      : Declaration(NK_FunctionDecl, std::move(Id), Init), Params(ParamList),
        WhenExpr(When) {}

  NodeList *getParamList() { return Params.get(); }
  FuncParamDecl *getParam(size_t I) {
    return Params->getNodeAs<FuncParamDecl>(I);
  }
  FuncParamDecl *getParam(size_t I) const {
    return Params->getNodeAs<FuncParamDecl>(I);
  }
  auto getParams() { return Params->getNodesAs<FuncParamDecl>(); }
  size_t getParamCount() const { return Params->size(); }

  auto getParamTypes() {
    return llvm::map_range(getParams(),
                           [](FuncParamDecl *Decl) { return Decl->getType(); });
  }

  bool isPureTemplate() { return getParamCount(); }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  FunctionDecl *clone() const override;
  Type *getType() override { assert(0 && "not implemented"); }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_FunctionDecl;
  }
};

class FunctionCall : public Expression {
  std::unique_ptr<ASTNode> Callee;
  std::unique_ptr<NodeList> Args;
  CallableFunction *CalleeFunc = nullptr;

public:
  FunctionCall(ASTNode *C, NodeList *ArgList)
      : Expression(NK_FunctionCall), Callee(C), Args(ArgList) {}
  FunctionCall(ASTNode *C) : FunctionCall(C, new NodeList) {}

  ASTNode *getCallee() { return Callee.get(); }

  NodeList *getArgList() { return Args.get(); }
  const NodeList *getArgList() const { return Args.get(); }

  Expression *getArg(size_t I) { return Args->getNodeAs<Expression>(I); }
  auto getArgs() { return Args->getNodesAs<Expression>(); }
  size_t getArgCount() const { return Args->size(); }

  /* Set by Sema */
  void setCalleeFunc(CallableFunction *F) { CalleeFunc = F; }
  CallableFunction *getCalleeFunc() { return CalleeFunc; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  FunctionCall *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_FunctionCall;
  }
};

class BuiltinTypeExpr : public Expression {
public:
  BuiltinTypeExpr(BuiltinType *Ty)
      : Expression(NK_BuiltinTypeExpr, MetaType::getTypeOf(Ty)) {}

  BuiltinType *getReferencedType() const {
    return llvm::cast<BuiltinType>(
        llvm::cast<MetaType>(getType())->getReferencedType());
  }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  BuiltinTypeExpr *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_BuiltinTypeExpr;
  }
};

class CallableFunction : private NonCopyable {
public:
  enum CallableType { CF_TemplateInstance, CF_ExternFunction };

private:
  CallableType CT;
  llvm::Function *Callee;

public:
  CallableFunction(CallableType T) : CT(T) {}
  virtual ~CallableFunction() {}
  virtual const std::string &getId() const = 0;

  CallableType getCallableType() const { return CT; }

  void setCallee(llvm::Function *C) { Callee = C; }
  llvm::Function *getCallee() { return Callee; }
};

/* Instance of potentially template, potentially multi-domain function.
 * Created by Sema, ready for Codegen */
class TemplateInstance : public CallableFunction {
  std::vector<std::unique_ptr<FunctionDecl>> Domains;
  std::string MangledId;

public:
  TemplateInstance(FunctionDecl *Domain)
      : CallableFunction(CF_TemplateInstance), MangledId(Domain->getId()) {
    appendDomain(Domain);
  }
  void appendDomain(FunctionDecl *Decl) { Domains.emplace_back(Decl); }
  auto getDomains() {
    return llvm::map_range(Domains, [](auto &&D) { return D.get(); });
  }
  FunctionDecl *getDomain(size_t Idx) { return Domains[Idx].get(); }
  bool isMultiDomain() const { return Domains.size() > 1; }
  auto getParamTypes() { return Domains.front()->getParamTypes(); }

  /* Definitions on all domains must have the same return type */
  Type *getReturnType() { return Domains.front()->getInitializer()->getType(); }

  // TODO: build mangled id based on parameter types
  void setIdPrefix(const std::string &P) { MangledId = P + MangledId; }
  const std::string &getId() const override { return MangledId; }

  static bool classof(const CallableFunction *F) {
    return F->getCallableType() == CF_TemplateInstance;
  }
};

/* Emitted by Sema to handle extern-expression-s */
class ExternFunction : public CallableFunction {
public:
  static bool classof(const CallableFunction *F) {
    return F->getCallableType() == CF_ExternFunction;
  }
};

}; // namespace mercy
