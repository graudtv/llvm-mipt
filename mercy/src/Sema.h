#pragma once

#include "AST.h"
#include <unordered_map>

namespace mercy {

class Sema {
  std::unordered_map<std::string, Declaration *> Decls;

public:
  void actOnBinaryOperator(BinaryOperator *BinOp);
  void actOnUnaryOperator(UnaryOperator *Op);
  void actOnFunctionCall(FunctionCall *FC);
  void actOnDeclaration(Declaration *Decl);
  void actOnIdentifier(Identifier *Id);

  void run(ASTNode *TU);
};

} // 
