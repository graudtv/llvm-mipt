#pragma once

#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

namespace llvm {
class Value;
}

namespace mercy {

class Codegen;

class ASTNode {
public:
  enum NodeKind {
    NK_IntegralLiteral,
    NK_BinaryOperator,
    NK_UnaryOperator,
    NK_Identifier,
    NK_FunctionCall,
    NK_ExpressionList
  };

private:
  NodeKind NK;

public:
  ASTNode(NodeKind K) : NK(K) {}
  virtual ~ASTNode() {}

  NodeKind getNodeKind() const { return NK; }
  virtual void print(llvm::raw_ostream &Os, unsigned Shift) const = 0;
  virtual llvm::Value *codegen(Codegen &Gen) = 0;

  void print(llvm::raw_ostream &Os = llvm::outs()) const { print(Os, 0); }
};

class IntegralLiteral : public ASTNode {
  int Value;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  IntegralLiteral(int V) : ASTNode(NK_IntegralLiteral), Value(V) {}
  int getValue() const { return Value; }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_IntegralLiteral;
  }
};

class BinaryOperator : public ASTNode {
public:
  enum BinOpKind { ADD, SUB, MUL, DIV, REM };

private:
  BinOpKind Kind;
  std::unique_ptr<ASTNode> LHS;
  std::unique_ptr<ASTNode> RHS;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  BinaryOperator(BinOpKind K, ASTNode *L, ASTNode *R)
      : ASTNode(NK_BinaryOperator), Kind(K), LHS(L), RHS(R) {}

  ASTNode *getLHS() { return LHS.get(); }
  ASTNode *getRHS() { return RHS.get(); }
  BinOpKind getKind() const { return Kind; }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_BinaryOperator;
  }
};

class UnaryOperator : public ASTNode {
public:
  enum UnaryOpKind { NEG };

private:
  UnaryOpKind Kind;
  std::unique_ptr<ASTNode> Expr;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  UnaryOperator(UnaryOpKind K, ASTNode *N)
      : ASTNode(NK_UnaryOperator), Kind(K), Expr(N) {}

  ASTNode *getExpr() { return Expr.get(); }
  UnaryOpKind getKind() const { return Kind; }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_UnaryOperator;
  }
};

class Identifier : public ASTNode {
  std::string Name;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  Identifier(std::string Id) : ASTNode(NK_Identifier), Name(std::move(Id)) {}
  const std::string &getName() const { return Name; }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_Identifier;
  }
};

class ExpressionList : public ASTNode {
  std::vector<std::unique_ptr<ASTNode>> Exprs;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  ExpressionList() : ASTNode(NK_ExpressionList) {}
  ExpressionList(ASTNode *N) : ASTNode(NK_ExpressionList) { append(N); }

  void append(ASTNode *N) { Exprs.emplace_back(N); }

  size_t size() const { return Exprs.size(); }
  ASTNode *getExpr(size_t Idx) { return Exprs[Idx].get(); }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_ExpressionList;
  }
};

class FunctionCall : public ASTNode {
  std::unique_ptr<ASTNode> Callee;
  std::unique_ptr<ASTNode> Args;

  void print(llvm::raw_ostream &Os, unsigned Shift) const override;
  llvm::Value *codegen(Codegen &Gen) override;

public:
  FunctionCall(ASTNode *C, ASTNode *ExprList)
      : ASTNode(NK_FunctionCall), Callee(C), Args(ExprList) {}
  FunctionCall(ASTNode *C) : FunctionCall(C, new ExpressionList) {}

  ASTNode *getCallee() { return Callee.get(); }

  ExpressionList *getArgList() {
    return llvm::cast<ExpressionList>(Args.get());
  }
  const ExpressionList *getArgList() const {
    return llvm::cast<ExpressionList>(Args.get());
  }
  ASTNode *getArg(size_t Idx) { return getArgList()->getExpr(Idx); }
  size_t getArgCount() const { return getArgList()->size(); }

  static bool classof(const ASTNode *N) {
    return N->getNodeKind() == NK_FunctionCall;
  }
};

}; // namespace mercy
