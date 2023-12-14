#pragma once

#include <list>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

namespace mercy {

class Type {
public:
  enum TypeKind { TK_BuiltinType, TK_ArrayType, TK_FunctionType, TK_MetaType };

private:
  TypeKind TK;

public:
  Type(TypeKind K) : TK(K) {}
  virtual ~Type() {}
  virtual void print(llvm::raw_ostream &Os) const = 0;
  void dump() const { print(llvm::outs()); }

  /* Returns LLVM type which can be used for passing expression of this type.
   * For BuiltinType, association is trivial
   * For ArrayType, type is pointer on element */
  virtual llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const = 0;

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
    Uint64,
    String
  };

private:
  BuiltinKind BK;
  BuiltinType(BuiltinKind K) : Type(TK_BuiltinType), BK(K) {}

  static BuiltinType VoidTy;
  static BuiltinType BoolTy;
  static BuiltinType Int8Ty, Int16Ty, Int32Ty, Int64Ty;
  static BuiltinType Uint8Ty, Uint16Ty, Uint32Ty, Uint64Ty;
  static BuiltinType StringTy;

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
  static BuiltinType *getStringTy() { return &StringTy; }

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
  bool isString() const { return BK == String; }

  bool isSigned() const;
  bool isUnsigned() const;
  bool isInteger() const { return isSigned() || isUnsigned(); }

  void print(llvm::raw_ostream &Os) const override;
  llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const override;

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
  llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const override;
  static bool classof(const Type *T) { return T->isArrayType(); }
};

class FunctionType : public Type {
  Type *RetTy;
  std::vector<Type *> ParamTys;

  // not very efficient storage, but at least it works
  static std::list<FunctionType> FuncTypes;

  FunctionType(Type *Result, llvm::ArrayRef<Type *> Params)
      : Type(TK_FunctionType), RetTy(Result),
        ParamTys(Params.begin(), Params.end()) {}

public:
  static FunctionType *get(Type *Result, llvm::ArrayRef<Type *> Params);

  Type *getReturnType() const { return RetTy; }
  Type *getParamType(size_t I) const { return ParamTys[I]; }
  const auto &getParamTypes() const { return ParamTys; }
  size_t getParamCount() const { return ParamTys.size(); }

  void print(llvm::raw_ostream &Os) const override;
  llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const override;
  static bool classof(const Type *T) { return T->isFunctionType(); }
};

/* MetaType is type of all Types */
class MetaType : public Type {
  static MetaType Instance;
  MetaType() : Type(TK_MetaType){};

public:
  static MetaType *get() { return &Instance; }

  void print(llvm::raw_ostream &Os) const override;
  llvm::Type *getLLVMType(llvm::LLVMContext &Ctx) const override;
  static bool classof(const Type *T) { return T->isMetaType(); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, const Type &T) {
  T.print(Os);
  return Os;
}

} // namespace mercy
