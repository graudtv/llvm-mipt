#include "sim.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

cl::opt<bool> RunOption("run", cl::desc("run code with llvm execution engine"));

void CreateColorScheme(Module &M) {
  LLVMContext &Ctx = M.getContext();
  Type *Int32Ty = IntegerType::getInt32Ty(Ctx);
  ArrayType *ArrayTy = ArrayType::get(Int32Ty, 9);
  Constant *Colors[] = {
      ConstantInt::get(Int32Ty, SIM_RGB(240, 223, 36)),
      ConstantInt::get(Int32Ty, SIM_RGB(98, 240, 36)),
      ConstantInt::get(Int32Ty, SIM_RGB(36, 240, 214)),
      ConstantInt::get(Int32Ty, SIM_RGB(36, 194, 240)),
      ConstantInt::get(Int32Ty, SIM_RGB(36, 50, 240)),
      ConstantInt::get(Int32Ty, SIM_RGB(194, 236, 240)),
      ConstantInt::get(Int32Ty, SIM_RGB(240, 36, 28)),
      ConstantInt::get(Int32Ty, SIM_RGB(240, 60, 36)),
      ConstantInt::get(Int32Ty, SIM_RGB(240, 122, 36)),
  };
  Constant *Array = ConstantArray::get(ArrayTy, Colors);

  /* @color_scheme = private global [9 x i32] [i32 VALUES...] */
  M.getOrInsertGlobal("color_scheme", ArrayTy);
  GlobalVariable *Var = M.getNamedGlobal("color_scheme");
  Var->setLinkage(GlobalVariable::PrivateLinkage);
  Var->setInitializer(Array);
}

void DeclareBuiltins(Module &M) {
  LLVMContext &Ctx = M.getContext();
  Type *VoidTy = Type::getVoidTy(Ctx);
  Type *I32 = Type::getInt32Ty(Ctx);
  M.getOrInsertFunction("sim_clear", VoidTy, I32);
  M.getOrInsertFunction("sim_display", VoidTy);
  M.getOrInsertFunction("sim_set_pixel", VoidTy, I32, I32, I32, I32);
  M.getOrInsertFunction("sim_rand", I32);
}

void DeclareFunctions(Module &M) {
  LLVMContext &Ctx = M.getContext();
  Type *VoidTy = Type::getVoidTy(Ctx);
  Type *I32 = Type::getInt32Ty(Ctx);

  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");
  M.getOrInsertFunction("update_neighbour_count", VoidTy,
                        MapStructTy->getPointerTo());
  M.getOrInsertFunction("init_map", VoidTy, MapStructTy->getPointerTo());
  M.getOrInsertFunction("draw_map", VoidTy, MapStructTy->getPointerTo());
  M.getOrInsertFunction("update_map", VoidTy, MapStructTy->getPointerTo());

#if 0
  /* for debugging */
  M.getOrInsertFunction("printf", FunctionType::get(I32, {}, true));
  M.getOrInsertFunction("puts", I32, Type::getInt8Ty(Ctx)->getPointerTo());
#endif
}

void CreateUpdateNeighbourCountFunc(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> Builder(Ctx);
  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");

  /* define void @update_neighbour_count(%struct.map_t *%map) { */
  Function *UpdateNeigbourCount = M.getFunction("update_neighbour_count");
  Value *MapStruct = UpdateNeigbourCount->getArg(0);
  MapStruct->setName("map");
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", UpdateNeigbourCount);
  BasicBlock *BBCondI = BasicBlock::Create(Ctx, "cond_i", UpdateNeigbourCount);
  BasicBlock *BBCondJ = BasicBlock::Create(Ctx, "cond_j", UpdateNeigbourCount);
  BasicBlock *BBLoopI = BasicBlock::Create(Ctx, "loop_i", UpdateNeigbourCount);
  BasicBlock *BBLoopJ = BasicBlock::Create(Ctx, "loop_j", UpdateNeigbourCount);
  BasicBlock *BBExitI = BasicBlock::Create(Ctx, "exit_i", UpdateNeigbourCount);
  BasicBlock *BBExitJ = BasicBlock::Create(Ctx, "exit_j", UpdateNeigbourCount);

  Builder.SetInsertPoint(BBEntry);
  Value *I = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i");
  Value *J = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "j");
  Builder.CreateStore(Builder.getInt32(0), I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBCondI);
  Value *IVal = Builder.CreateLoad(Builder.getInt32Ty(), I, "ival");
  Value *ICmp =
      Builder.CreateICmpSLT(IVal, Builder.getInt32(SIM_SCREEN_HEIGHT), "icmp");
  Builder.CreateCondBr(ICmp, BBLoopI, BBExitI);

  Builder.SetInsertPoint(BBLoopI);
  Builder.CreateStore(Builder.getInt32(0), J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBCondJ);
  Value *JVal = Builder.CreateLoad(Builder.getInt32Ty(), J, "jval");
  Value *JCmp =
      Builder.CreateICmpSLT(JVal, Builder.getInt32(SIM_SCREEN_WIDTH), "jcmp");
  Builder.CreateCondBr(JCmp, BBLoopJ, BBExitJ);

  Builder.SetInsertPoint(BBLoopJ);
  /* left_idx = (j + MAP_WIDTH - 1) % MAP_WIDTH; */
  Value *Left = Builder.CreateAdd(JVal, Builder.getInt32(SIM_SCREEN_WIDTH - 1),
                                  "left_idx.tmp");
  Value *LeftIdx =
      Builder.CreateURem(Left, Builder.getInt32(SIM_SCREEN_WIDTH), "left_idx");
  /* right_idx = (j + 1) % MAP_WIDTH; */
  Value *Right = Builder.CreateAdd(JVal, Builder.getInt32(1), "right_idx.tmp");
  Value *RightIdx = Builder.CreateURem(
      Right, Builder.getInt32(SIM_SCREEN_WIDTH), "right_idx");
  /* top_idx = (i + MAP_HEIGHT - 1) % MAP_HEIGHT; */
  Value *Top = Builder.CreateAdd(IVal, Builder.getInt32(SIM_SCREEN_HEIGHT - 1),
                                 "top_idx.tmp");
  Value *TopIdx =
      Builder.CreateURem(Top, Builder.getInt32(SIM_SCREEN_HEIGHT), "top_idx");
  /* bottom_idx = (i + 1) % MAP_HEIGHT; */
  Value *Bottom =
      Builder.CreateAdd(IVal, Builder.getInt32(1), "bottom_idx.tmp");
  Value *BottomIdx = Builder.CreateURem(
      Bottom, Builder.getInt32(SIM_SCREEN_HEIGHT), "bottom_idx");
  Value *Zero = Builder.getInt32(0);
  /* fetch m->is_alive[i][j] for all 9 indices */
  Value *ValTopLeft = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, TopIdx, LeftIdx}, "top_left_ptr"),
      "top_left");
  Value *ValTop = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, TopIdx, JVal}, "top_ptr"),
      "top");
  Value *ValTopRight =
      Builder.CreateLoad(Builder.getInt8Ty(),
                         Builder.CreateInBoundsGEP(
                             MapStructTy, MapStruct,
                             {Zero, Zero, TopIdx, RightIdx}, "top_right_ptr"),
                         "top_right");
  Value *ValLeft = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, IVal, LeftIdx}, "left_ptr"),
      "center_left");
  Value *ValRight = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, IVal, RightIdx}, "right_ptr"),
      "right");
  Value *ValBottomLeft = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, BottomIdx, LeftIdx},
                                "bottom_left_ptr"),
      "bottom_left");
  Value *ValBottom = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, BottomIdx, JVal}, "bottom_ptr"),
      "bottom");
  Value *ValBottomRight = Builder.CreateLoad(
      Builder.getInt8Ty(),
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct,
                                {Zero, Zero, BottomIdx, RightIdx},
                                "bottom_right_ptr"),
      "bottom_right");
  /* Calculate sum of 8 elems */
  Value *Tmp1 = Builder.CreateAdd(ValTopLeft, ValTop, "tmp.1");
  Value *Tmp2 = Builder.CreateAdd(ValTopRight, ValLeft, "tmp.2");
  Value *Tmp12 = Builder.CreateAdd(Tmp1, Tmp2, "tmp.12");
  Value *Tmp3 = Builder.CreateAdd(ValRight, ValBottomLeft, "tmp.3");
  Value *Tmp4 = Builder.CreateAdd(ValBottom, ValBottomRight, "tmp.4");
  Value *Tmp34 = Builder.CreateAdd(Tmp3, Tmp4, "tmp.34");
  Value *NCount = Builder.CreateAdd(Tmp12, Tmp34, "neighbour_count");
  /* m->neighbour_count[i][j] = ncount; */
  Value *NIdxList[] = {Builder.getInt32(0), Builder.getInt32(1), IVal, JVal};
  Value *NCountPtr = Builder.CreateInBoundsGEP(MapStructTy, MapStruct, NIdxList,
                                               "neighbour_count_ptr");
  Builder.CreateStore(NCount, NCountPtr);
  Value *JValInc = Builder.CreateAdd(JVal, Builder.getInt32(1), "jval.inc");
  Builder.CreateStore(JValInc, J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBExitJ);
  Value *IValInc = Builder.CreateAdd(IVal, Builder.getInt32(1), "ival.inc");
  Builder.CreateStore(IValInc, I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBExitI);
  Builder.CreateRetVoid();
  /* } // update_neighbour_count() */
}

void CreateUpdateMapFunc(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> Builder(Ctx);
  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");

  /* define void @update_map(%struct.map_t *%map) { */
  Function *UpdateMap = M.getFunction("update_map");
  Value *MapStruct = UpdateMap->getArg(0);
  MapStruct->setName("map");
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", UpdateMap);
  BasicBlock *BBCondI = BasicBlock::Create(Ctx, "cond_i", UpdateMap);
  BasicBlock *BBCondJ = BasicBlock::Create(Ctx, "cond_j", UpdateMap);
  BasicBlock *BBLoopI = BasicBlock::Create(Ctx, "loop_i", UpdateMap);
  BasicBlock *BBLoopJ = BasicBlock::Create(Ctx, "loop_j", UpdateMap);
  BasicBlock *BBExitI = BasicBlock::Create(Ctx, "exit_i", UpdateMap);
  BasicBlock *BBExitJ = BasicBlock::Create(Ctx, "exit_j", UpdateMap);

  Builder.SetInsertPoint(BBEntry);
  Value *I = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i");
  Value *J = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "j");
  Builder.CreateStore(Builder.getInt32(0), I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBCondI);
  Value *IVal = Builder.CreateLoad(Builder.getInt32Ty(), I, "ival");
  Value *ICmp =
      Builder.CreateICmpSLT(IVal, Builder.getInt32(SIM_SCREEN_HEIGHT), "icmp");
  Builder.CreateCondBr(ICmp, BBLoopI, BBExitI);

  Builder.SetInsertPoint(BBLoopI);
  Builder.CreateStore(Builder.getInt32(0), J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBCondJ);
  Value *JVal = Builder.CreateLoad(Builder.getInt32Ty(), J, "jval");
  Value *JCmp =
      Builder.CreateICmpSLT(JVal, Builder.getInt32(SIM_SCREEN_WIDTH), "jcmp");
  Builder.CreateCondBr(JCmp, BBLoopJ, BBExitJ);

  Builder.SetInsertPoint(BBLoopJ);
  Value *IdxList[] = {Builder.getInt32(0), Builder.getInt32(0), IVal, JVal};
  Value *IsAlivePtr = Builder.CreateInBoundsGEP(MapStructTy, MapStruct, IdxList,
                                                "is_alive_ptr");
  Value *IsAlive =
      Builder.CreateLoad(Builder.getInt8Ty(), IsAlivePtr, "is_alive");
  Value *AliveBit = Builder.CreateICmpNE(IsAlive, Builder.getInt8(0), "alive");
  Value *NotAliveBit = Builder.CreateNot(AliveBit, "not_alive");

  Value *NIdxList[] = {Builder.getInt32(0), Builder.getInt32(1), IVal, JVal};
  Value *NCountPtr = Builder.CreateInBoundsGEP(MapStructTy, MapStruct, NIdxList,
                                               "neighbour_count_ptr");
  Value *NCount =
      Builder.CreateLoad(Builder.getInt8Ty(), NCountPtr, "neighbour_count");
  Value *NCountEq2 =
      Builder.CreateICmpEQ(NCount, Builder.getInt8(2), "ncount_eq_2");
  Value *NCountEq3 =
      Builder.CreateICmpEQ(NCount, Builder.getInt8(3), "ncount_eq_3");
  /* Check all 3 conditions when cell remain/becomes alive */
  Value *Cond1 = Builder.CreateAnd(AliveBit, NCountEq2, "cond1");
  Value *Cond2 = Builder.CreateAnd(AliveBit, NCountEq3, "cond2");
  Value *Cond3 = Builder.CreateAnd(NotAliveBit, NCountEq3, "cond3");
  /* m->is_alive[i][j]  = Cond1 || Cond2 || Cond3 */
  Value *CondTmp = Builder.CreateOr(Cond1, Cond2, "tmp");
  Value *Cond = Builder.CreateOr(CondTmp, Cond3, "cond");
  Value *SetAlive = Builder.CreateZExt(Cond, Builder.getInt8Ty(), "set_alive");
  Builder.CreateStore(SetAlive, IsAlivePtr);
  /* increment j */
  Value *JValInc = Builder.CreateAdd(JVal, Builder.getInt32(1), "jval.inc");
  Builder.CreateStore(JValInc, J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBExitJ);
  Value *IValInc = Builder.CreateAdd(IVal, Builder.getInt32(1), "ival.inc");
  Builder.CreateStore(IValInc, I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBExitI);
  Builder.CreateCall(M.getFunction("update_neighbour_count"), MapStruct);
  Builder.CreateRetVoid();
  /* } // update_map() */
}

void CreateInitMapFunc(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> Builder(Ctx);
  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");

  /* define void @init_map(%struct.map_t *%map) { */
  Function *InitMap = M.getFunction("init_map");
  Value *MapStruct = InitMap->getArg(0);
  MapStruct->setName("map");
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", InitMap);
  BasicBlock *BBCondI = BasicBlock::Create(Ctx, "cond_i", InitMap);
  BasicBlock *BBCondJ = BasicBlock::Create(Ctx, "cond_j", InitMap);
  BasicBlock *BBLoopI = BasicBlock::Create(Ctx, "loop_i", InitMap);
  BasicBlock *BBLoopJ = BasicBlock::Create(Ctx, "loop_j", InitMap);
  BasicBlock *BBExitI = BasicBlock::Create(Ctx, "exit_i", InitMap);
  BasicBlock *BBExitJ = BasicBlock::Create(Ctx, "exit_j", InitMap);

  Builder.SetInsertPoint(BBEntry);
  Value *I = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i");
  Value *J = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "j");
  Builder.CreateStore(Builder.getInt32(0), I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBCondI);
  Value *IVal = Builder.CreateLoad(Builder.getInt32Ty(), I, "ival");
  Value *ICmp =
      Builder.CreateICmpSLT(IVal, Builder.getInt32(SIM_SCREEN_HEIGHT), "icmp");
  Builder.CreateCondBr(ICmp, BBLoopI, BBExitI);

  Builder.SetInsertPoint(BBLoopI);
  Builder.CreateStore(Builder.getInt32(0), J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBCondJ);
  Value *JVal = Builder.CreateLoad(Builder.getInt32Ty(), J, "jval");
  Value *JCmp =
      Builder.CreateICmpSLT(JVal, Builder.getInt32(SIM_SCREEN_WIDTH), "jcmp");
  Builder.CreateCondBr(JCmp, BBLoopJ, BBExitJ);

  Builder.SetInsertPoint(BBLoopJ);
  Value *Rand = Builder.CreateCall(M.getFunction("sim_rand"), {}, "rand");
  Value *SRem = Builder.CreateSRem(Rand, Builder.getInt32(5), "srem");
  Value *Cmp = Builder.CreateICmpEQ(SRem, Builder.getInt32(0), "is_alive");
  Value *IsAlive = Builder.CreateZExt(Cmp, Builder.getInt8Ty(), "is_alive.i8");
  Value *IdxList[] = {Builder.getInt32(0), Builder.getInt32(0), IVal, JVal};
  Value *Elem =
      Builder.CreateInBoundsGEP(MapStructTy, MapStruct, IdxList, "elem");
  Builder.CreateStore(IsAlive, Elem);
  Value *JValInc = Builder.CreateAdd(JVal, Builder.getInt32(1), "jval.inc");
  Builder.CreateStore(JValInc, J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBExitJ);
  Value *IValInc = Builder.CreateAdd(IVal, Builder.getInt32(1), "ival.inc");
  Builder.CreateStore(IValInc, I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBExitI);
  Builder.CreateCall(M.getFunction("update_neighbour_count"), MapStruct);
  Builder.CreateRetVoid();
  /* } // init_map() */
}

void CreateDrawMapFunc(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> Builder(Ctx);
  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");

  /* define void @draw_map(%struct.map_t *%map) { */
  Function *DrawMap = M.getFunction("draw_map");
  Value *MapStruct = DrawMap->getArg(0);
  MapStruct->setName("map");
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", DrawMap);
  BasicBlock *BBCondI = BasicBlock::Create(Ctx, "cond_i", DrawMap);
  BasicBlock *BBCondJ = BasicBlock::Create(Ctx, "cond_j", DrawMap);
  BasicBlock *BBLoopI = BasicBlock::Create(Ctx, "loop_i", DrawMap);
  BasicBlock *BBLoopJ = BasicBlock::Create(Ctx, "loop_j", DrawMap);
  BasicBlock *BBExitI = BasicBlock::Create(Ctx, "exit_i", DrawMap);
  BasicBlock *BBExitJ = BasicBlock::Create(Ctx, "exit_j", DrawMap);
  BasicBlock *BBIncJ = BasicBlock::Create(Ctx, "inc_j", DrawMap);
  BasicBlock *BBDraw = BasicBlock::Create(Ctx, "draw", DrawMap);

  Builder.SetInsertPoint(BBEntry);
  Value *I = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i");
  Value *J = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "j");
  Builder.CreateStore(Builder.getInt32(0), I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBCondI);
  Value *IVal = Builder.CreateLoad(Builder.getInt32Ty(), I, "ival");
  Value *ICmp =
      Builder.CreateICmpSLT(IVal, Builder.getInt32(SIM_SCREEN_HEIGHT), "icmp");
  Builder.CreateCondBr(ICmp, BBLoopI, BBExitI);

  Builder.SetInsertPoint(BBLoopI);
  Builder.CreateStore(Builder.getInt32(0), J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBCondJ);
  Value *JVal = Builder.CreateLoad(Builder.getInt32Ty(), J, "jval");
  Value *JCmp =
      Builder.CreateICmpSLT(JVal, Builder.getInt32(SIM_SCREEN_WIDTH), "jcmp");
  Builder.CreateCondBr(JCmp, BBLoopJ, BBExitJ);

  Builder.SetInsertPoint(BBLoopJ);
  Value *IdxList[] = {Builder.getInt32(0), Builder.getInt32(0), IVal, JVal};
  Value *IsAlivePtr = Builder.CreateInBoundsGEP(MapStructTy, MapStruct, IdxList,
                                                "is_alive_ptr");
  Value *IsAlive =
      Builder.CreateLoad(Builder.getInt8Ty(), IsAlivePtr, "is_alive");
  Value *Cmp = Builder.CreateICmpNE(IsAlive, Builder.getInt8(0), "cmp");
  Builder.CreateCondBr(Cmp, BBDraw, BBIncJ);

  Builder.SetInsertPoint(BBDraw);
  Value *NIdxList[] = {Builder.getInt32(0), Builder.getInt32(1), IVal, JVal};
  Value *NCountPtr = Builder.CreateInBoundsGEP(MapStructTy, MapStruct, NIdxList,
                                               "neighbour_count_ptr");
  Value *NCount =
      Builder.CreateLoad(Builder.getInt8Ty(), NCountPtr, "neighbour_count");
  Value *CSIdxList[] = {Builder.getInt32(0), NCount};
  GlobalVariable *CS = M.getNamedGlobal("color_scheme");
  Value *ColorPtr = Builder.CreateInBoundsGEP(
      ArrayType::get(Builder.getInt32Ty(), 9), CS, CSIdxList, "color_ptr");
  Value *Color = Builder.CreateLoad(Builder.getInt32Ty(), ColorPtr, "color");
  Value *CallArgs[] = {JVal, IVal, Color, Builder.getInt32(SIM_CIRCLE_SHAPE)};
  Builder.CreateCall(M.getFunction("sim_set_pixel"), CallArgs);
  Builder.CreateBr(BBIncJ);

  Builder.SetInsertPoint(BBIncJ);
  Value *JValInc = Builder.CreateAdd(JVal, Builder.getInt32(1), "jval.inc");
  Builder.CreateStore(JValInc, J);
  Builder.CreateBr(BBCondJ);

  Builder.SetInsertPoint(BBExitJ);
  Value *IValInc = Builder.CreateAdd(IVal, Builder.getInt32(1), "ival.inc");
  Builder.CreateStore(IValInc, I);
  Builder.CreateBr(BBCondI);

  Builder.SetInsertPoint(BBExitI);
  Builder.CreateCall(M.getFunction("update_neighbour_count"), MapStruct);
  Builder.CreateRetVoid();
  /* } // draw_map() */
}

void CreateMainFunc(Module &M) {
  LLVMContext &Ctx = M.getContext();
  IRBuilder<> Builder(Ctx);
  Type *MapStructTy = StructType::getTypeByName(Ctx, "struct.map_t");

  /* define i32 @main() { */
  FunctionType *MainTy = llvm::FunctionType::get(Builder.getInt32Ty(), false);
  Function *Main =
      Function::Create(MainTy, Function::ExternalLinkage, "main", M);
  BasicBlock *BBEntry = BasicBlock::Create(Ctx, "entry", Main);
  BasicBlock *BBLoop = BasicBlock::Create(Ctx, "loop", Main);

  Builder.SetInsertPoint(BBEntry);
  Value *MapStruct = Builder.CreateAlloca(MapStructTy, nullptr, "map");
  Builder.CreateCall(M.getFunction("init_map"), MapStruct);
  Builder.CreateBr(BBLoop);

  Builder.SetInsertPoint(BBLoop);
  Builder.CreateCall(M.getFunction("sim_clear"),
                     Builder.getInt32(SIM_RGB(128, 128, 128)));
  Builder.CreateCall(M.getFunction("draw_map"), MapStruct);
  Builder.CreateCall(M.getFunction("sim_display"));
  Builder.CreateCall(M.getFunction("update_map"), MapStruct);
  Builder.CreateBr(BBLoop);
  /* } // main() */
}

void *LazyFunctionCreator(const std::string &FuncName) {
  return StringSwitch<void *>(FuncName)
      .Case("sim_clear", reinterpret_cast<void *>(sim_clear))
      .Case("sim_display", reinterpret_cast<void *>(sim_display))
      .Case("sim_set_pixel", reinterpret_cast<void *>(sim_set_pixel))
      .Case("sim_rand", reinterpret_cast<void *>(sim_rand))
      .Default(nullptr);
}

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv);

  LLVMContext Ctx;
  Module *M = new Module("top", Ctx);
  IRBuilder<> Builder(Ctx);

  Type *MapRowTy = ArrayType::get(Builder.getInt8Ty(), SIM_SCREEN_WIDTH);
  Type *MapTy = ArrayType::get(MapRowTy, SIM_SCREEN_HEIGHT);
  /* %struct.map_t = type { [SIM_SCREEN_HEIGHT x [SIM_SCREEN_WIDTH x i8]],
   *                        [SIM_SCREEN_HEIGHT x [SIM_SCREEN_WIDTH x i8]] }
   */
  Type *MapStructTy = StructType::create(Ctx, {MapTy, MapTy}, "struct.map_t");

  DeclareBuiltins(*M);
  DeclareFunctions(*M);
  CreateColorScheme(*M);
  CreateUpdateNeighbourCountFunc(*M);
  CreateDrawMapFunc(*M);
  CreateUpdateMapFunc(*M);
  CreateInitMapFunc(*M);
  CreateMainFunc(*M);

  outs() << *M;
  verifyModule(*M, &outs());

  if (RunOption) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    ExecutionEngine *EE = EngineBuilder(std::unique_ptr<Module>(M)).create();
    EE->InstallLazyFunctionCreator(LazyFunctionCreator);
    EE->finalizeObject();
    if (EE->hasError()) {
      outs() << "\nEngine error: " << EE->getErrorMessage() << "\n";
      return 1;
    }
    outs() << "\nRunning code...\n";
    int RetCode = EE->runFunctionAsMain(M->getFunction("main"), {}, nullptr);
    outs() << "RETCODE " << RetCode << "\n";
  }
  return 0;
}
