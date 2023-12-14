#pragma once

#include "misc.h"
#include <llvm/ADT/Any.h>
#include <string>

namespace mercy {

class Type;
class Declaration;
class ExternFunction;

class GenericValue {
  llvm::Any Value;
  GenericValue(llvm::Any V) : Value(std::move(V)) {}

public:
  GenericValue() = default;

  static GenericValue makeInt(int V) { return GenericValue(V); }
  static GenericValue makeString(const std::string &V) {
    return GenericValue(V);
  }
  static GenericValue makeDeclRef(Declaration *D) { return GenericValue(D); }
  static GenericValue makeExternFuncRef(ExternFunction *F) {
    return GenericValue(F);
  }
  static GenericValue makeType(Type *V) { return GenericValue(V); }

  bool isEmpty() const { return !Value.hasValue(); }
  bool isInt() const { return llvm::any_isa<int>(Value); }
  bool isString() const { return llvm::any_isa<std::string>(Value); }
  bool isDeclRef() const { return llvm::any_isa<Declaration *>(Value); }
  bool isExternFuncRef() const { return llvm::any_isa<ExternFunction *>(Value); }
  bool isType() const { return llvm::any_isa<Type *>(Value); }

  int asInt() const { return llvm::any_cast<int>(Value); }
  const std::string &asString() const {
    return *llvm::any_cast<std::string>(&Value);
  }
  const Declaration *asDeclRef() const { return llvm::any_cast<Declaration *>(Value); }
  const ExternFunction *asExternFuncRef() const {
    return llvm::any_cast<ExternFunction *>(Value);
  }
  Type *asType() const { return llvm::any_cast<Type *>(Value); }

  operator bool() const { return !isEmpty(); }
};

} // namespace mercy
