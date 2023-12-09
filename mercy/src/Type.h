#pragma once

#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

namespace mercy {

class Type {
public:
  enum TypeKind { TK_BuiltinType, TK_FunctionType, TK_ArrayType, TK_MetaType };

private:
  TypeKind TK;

public:
  Type(TypeKind K) : TK(K) {}
  virtual ~Type() {}
  virtual void print(llvm::raw_ostream &Os) const = 0;
  void print() const { print(llvm::outs()); }
  std::string toString() const;

  TypeKind getKind() const { return TK; }

  bool isBuiltinType() const { return TK == TK_BuiltinType; }
  bool isFunctionType() const { return TK == TK_FunctionType; }
  bool isArrayType() const { return TK == TK_ArrayType; }
  bool isMetaType() const { return TK == TK_MetaType; }
};

class BuiltinType : public Type {
public:
  enum BuiltinKind {
    Void,
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    Uint8,
    Uint16,
    Uint32,
    Uint64
  };

private:
  BuiltinKind BK;
  BuiltinType(BuiltinKind K) : Type(TK_BuiltinType), BK(K) {}

  static BuiltinType VoidTy;
  static BuiltinType BoolTy;
  static BuiltinType Int8Ty, Int16Ty, Int32Ty, Int64Ty;
  static BuiltinType Uint8Ty, Uint16Ty, Uint32Ty, Uint64Ty;

public:
  static BuiltinType *getVoidTy() { return &VoidTy; }
  static BuiltinType *getBoolTy() { return &BoolTy; }
  static BuiltinType *getInt8Ty() { return &Int8Ty; }
  static BuiltinType *getInt16Ty() { return &Int16Ty; }
  static BuiltinType *getInt32Ty() { return &Int32Ty; }
  static BuiltinType *getInt64Ty() { return &Int64Ty; }
  static BuiltinType *getUint8Ty() { return &Uint8Ty; }
  static BuiltinType *getUint16Ty() { return &Uint16Ty; }
  static BuiltinType *getUint32Ty() { return &Uint32Ty; }
  static BuiltinType *getUint64Ty() { return &Uint64Ty; }

  /* Type aliases */
  static BuiltinType *getIntTy() { return getInt32Ty(); }
  static BuiltinType *getUintTy() { return getUint32Ty(); }

  bool isVoid() const { return BK == Void; }
  bool isBool() const { return BK == Bool; }
  bool isInt8() const { return BK == Int8; }
  bool isInt16() const { return BK == Int16; }
  bool isInt32() const { return BK == Int32; }
  bool isInt64() const { return BK == Int64; }
  bool isUint8() const { return BK == Uint8; }
  bool isUint16() const { return BK == Uint16; }
  bool isUint32() const { return BK == Uint32; }
  bool isUint64() const { return BK == Uint64; }

  bool isSigned() const;
  bool isUnsigned() const;
  bool isInteger() const { return isSigned() || isUnsigned(); }

  llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const;

  void print(llvm::raw_ostream &Os) const override;

  static bool classof(const Type *T) { return T->isBuiltinType(); }
};

class ArrayType : public Type {
  Type *ElementTy;

  ArrayType(Type *ElemTy) : Type(TK_ArrayType), ElementTy(ElemTy) {}
  static std::unordered_map<Type *, ArrayType> ArrayTypes;

public:
  static ArrayType *get(Type *ElemTy);
  Type *getElemTy() { return ElementTy; }

  void print(llvm::raw_ostream &Os) const override;
  static bool classof(const Type *T) { return T->isArrayType(); }
};

/* MetaType is type of all Types */
class MetaType : public Type {
  static MetaType Instance;
  MetaType() : Type(TK_MetaType){};

public:
  static MetaType *get() { return &Instance; }

  void print(llvm::raw_ostream &Os) const override;
  static bool classof(const Type *T) { return T->isMetaType(); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, const Type &T) {
  T.print(Os);
  return Os;
}

} // namespace mercy
