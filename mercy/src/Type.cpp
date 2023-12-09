#include "Type.h"
#include <llvm/IR/DerivedTypes.h>

using namespace mercy;

namespace {

const char *getBuiltinTypeName(BuiltinType::BuiltinKind K) {
  // clang-format off
  switch (K) {
  case BuiltinType::Void: return "void";
  case BuiltinType::Bool: return "bool";
  case BuiltinType::Int8: return "int8";
  case BuiltinType::Int16: return "int16";
  case BuiltinType::Int32: return "int32";
  case BuiltinType::Int64: return "int64";
  case BuiltinType::Uint8: return "uint8";
  case BuiltinType::Uint16: return "uint16";
  case BuiltinType::Uint32: return "uint32";
  case BuiltinType::Uint64: return "uint64";
  }
  llvm_unreachable("invalid builtin type");
  // clang-format on
}

} // namespace

BuiltinType BuiltinType::VoidTy{BuiltinType::Void};
BuiltinType BuiltinType::BoolTy{BuiltinType::Bool};
BuiltinType BuiltinType::Int8Ty{BuiltinType::Int8};
BuiltinType BuiltinType::Int16Ty{BuiltinType::Int16};
BuiltinType BuiltinType::Int32Ty{BuiltinType::Int32};
BuiltinType BuiltinType::Int64Ty{BuiltinType::Int64};
BuiltinType BuiltinType::Uint8Ty{BuiltinType::Uint8};
BuiltinType BuiltinType::Uint16Ty{BuiltinType::Uint16};
BuiltinType BuiltinType::Uint32Ty{BuiltinType::Uint32};
BuiltinType BuiltinType::Uint64Ty{BuiltinType::Uint64};
std::unordered_map<Type *, ArrayType> ArrayType::ArrayTypes;
MetaType MetaType::Instance{};

std::string Type::toString() const {
  std::string Res;
  llvm::raw_string_ostream Os{Res};
  print(Os);
  return Res;
}

/* uintptr doesn't count as unsigned type, because it's opaque */
bool BuiltinType::isUnsigned() const {
  return BK == Uint8 || BK == Uint16 || BK == Uint32 || BK == Uint64;
}

bool BuiltinType::isSigned() const {
  return BK == Int8 || BK == Int16 || BK == Int32 || BK == Int64;
}

llvm::Type *BuiltinType::getLLVMType(llvm::LLVMContext &Ctx) const {
  switch (BK) {
  case Void:
    return llvm::Type::getVoidTy(Ctx);
  case Bool:
  case Int8:
  case Uint8:
    return llvm::Type::getInt8Ty(Ctx);
  case Int16:
  case Uint16:
    return llvm::Type::getInt16Ty(Ctx);
  case Int32:
  case Uint32:
    return llvm::Type::getInt32Ty(Ctx);
  case Int64:
  case Uint64:
    return llvm::Type::getInt64Ty(Ctx);
  }
  llvm_unreachable("unhandled type");
}

void BuiltinType::print(llvm::raw_ostream &Os) const {
  Os << getBuiltinTypeName(BK);
}

ArrayType *ArrayType::get(Type *T) {
  if (auto It = ArrayTypes.find(T); It != ArrayTypes.end())
    return &It->second;
  auto It = ArrayTypes.emplace(std::make_pair(T, ArrayType(T))).first;
  return &It->second;
}

void ArrayType::print(llvm::raw_ostream &Os) const {
  Os << "array_type(" << *ElementTy << ')';
}

void MetaType::print(llvm::raw_ostream &Os) const {
  Os << "MetaType";
}
