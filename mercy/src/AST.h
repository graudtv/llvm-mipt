#pragma once

#include "Type.h"
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

namespace llvm {
class Value;
}

namespace mercy {

class Codegen;
class Sema;
class Declaration;

struct Location {
  unsigned LineNo = 0;

  bool isValid() const { return LineNo != 0; }
};

class ASTNode {
public:
  enum NodeKind {
    NK_IntegralLiteral,
    NK_BinaryOperator,
    NK_UnaryOperator,
    NK_Identifier,
    NK_FunctionCall,
    NK_Declaration,
    NK_FunctionDeclaration,
    NK_NodeList,
    NK_BuiltinTypeExpr
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
  enum BinOpKind { ADD, SUB, MUL, DIV, REM };

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
  enum UnaryOpKind { NEG };

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

/* Variable, type alias or function parameter declaration */
class Declaration : public ASTNode {
  std::string Identifier;
  std::unique_ptr<Expression> Initializer;
  bool IsRef;
  llvm::Value *Alloca = nullptr;

public:
  Declaration(std::string Id, Expression *E, bool IsReference)
      : ASTNode(NK_Declaration), Identifier(std::move(Id)), Initializer(E),
        IsRef(IsReference) {}
  Declaration(std::string Id, bool IsReference)
      : Declaration(std::move(Id), nullptr, IsReference) {}

  const std::string &getId() const { return Identifier; }
  Expression *getInitializer() { return Initializer.get(); }
  bool isRef() const { return IsRef; }

  /* Set by Codegen */
  void setAlloca(llvm::Value *A) { Alloca = A; }
  llvm::Value *getAlloca() { return Alloca; }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  Declaration *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_Declaration;
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

class FunctionDeclaration : public ASTNode {
  std::string Identifier;
  std::unique_ptr<NodeList> Params;
  std::unique_ptr<Expression> Body;

public:
  FunctionDeclaration(std::string Id, NodeList *P, Expression *E)
      : ASTNode(NK_FunctionDeclaration), Identifier(std::move(Id)), Params(P),
        Body(E) {}

  const std::string &getId() const { return Identifier; }
  Expression *getBody() { return Body.get(); }

  NodeList *getParamList() { return Params.get(); }
  Declaration *getParam(size_t I) { return Params->getNodeAs<Declaration>(I); }
  auto getParams() { return Params->getNodesAs<Declaration>(); }
  size_t getParamCount() const { return Params->size(); }

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;
  void sema(Sema &S) override;
  FunctionDeclaration *clone() const override;

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_FunctionDeclaration;
  }
};

class FunctionCall : public Expression {
  std::unique_ptr<ASTNode> Callee;
  std::unique_ptr<NodeList> Args;

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

}; // namespace mercy
