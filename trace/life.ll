; ModuleID = '/home/vova/Documents/GitHub/llvm-mipt/trace/../life_game/src/life.c'
source_filename = "/home/vova/Documents/GitHub/llvm-mipt/trace/../life_game/src/life.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.map_t = type { [200 x [100 x i8]], [200 x [100 x i8]] }

@__const.draw_map.color_scheme = private unnamed_addr constant [9 x i32] [i32 -253811457, i32 1659905279, i32 619763455, i32 616755455, i32 607318271, i32 -1024659201, i32 -266068737, i32 -264493825, i32 -260430593], align 16

; Function Attrs: nounwind uwtable
define dso_local void @draw_map(%struct.map_t* nocapture noundef readonly %0) local_unnamed_addr #0 {
  br label %2

2:                                                ; preds = %1, %6
  %3 = phi i64 [ 0, %1 ], [ %7, %6 ]
  %4 = trunc i64 %3 to i32
  br label %9

5:                                                ; preds = %6
  ret void

6:                                                ; preds = %21
  %7 = add nuw nsw i64 %3, 1
  %8 = icmp eq i64 %7, 100
  br i1 %8, label %5, label %2, !llvm.loop !5

9:                                                ; preds = %2, %21
  %10 = phi i64 [ 0, %2 ], [ %22, %21 ]
  %11 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 %10
  %12 = load i8, i8* %11, align 1, !tbaa !7
  %13 = icmp eq i8 %12, 0
  br i1 %13, label %21, label %14

14:                                               ; preds = %9
  %15 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 %10
  %16 = load i8, i8* %15, align 1, !tbaa !7
  %17 = zext i8 %16 to i64
  %18 = getelementptr inbounds [9 x i32], [9 x i32]* @__const.draw_map.color_scheme, i64 0, i64 %17
  %19 = load i32, i32* %18, align 4, !tbaa !10
  %20 = trunc i64 %10 to i32
  tail call void @sim_set_pixel(i32 noundef %20, i32 noundef %4, i32 noundef %19, i32 noundef 32) #5
  br label %21

21:                                               ; preds = %9, %14
  %22 = add nuw nsw i64 %10, 1
  %23 = icmp eq i64 %22, 200
  br i1 %23, label %6, label %9, !llvm.loop !12
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare void @sim_set_pixel(i32 noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @update_neighbour_count(%struct.map_t* nocapture noundef %0) local_unnamed_addr #3 {
  br label %2

2:                                                ; preds = %1, %13
  %3 = phi i64 [ 0, %1 ], [ %7, %13 ]
  %4 = trunc i64 %3 to i32
  %5 = add i32 %4, 99
  %6 = urem i32 %5, 100
  %7 = add nuw nsw i64 %3, 1
  %8 = icmp eq i64 %7, 100
  %9 = zext i32 %6 to i64
  %10 = and i64 %7, 4294967295
  %11 = select i1 %8, i64 0, i64 %10
  br label %15

12:                                               ; preds = %13
  ret void

13:                                               ; preds = %15
  %14 = icmp eq i64 %7, 100
  br i1 %14, label %12, label %2, !llvm.loop !13

15:                                               ; preds = %2, %15
  %16 = phi i64 [ 0, %2 ], [ %20, %15 ]
  %17 = trunc i64 %16 to i16
  %18 = add i16 %17, 199
  %19 = urem i16 %18, 200
  %20 = add nuw nsw i64 %16, 1
  %21 = icmp ult i64 %16, 199
  %22 = add nuw i64 %16, 57
  %23 = select i1 %21, i64 %20, i64 %22
  %24 = and i64 %23, 255
  %25 = zext i16 %19 to i64
  %26 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %9, i64 %25
  %27 = load i8, i8* %26, align 1, !tbaa !7
  %28 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %9, i64 %16
  %29 = load i8, i8* %28, align 1, !tbaa !7
  %30 = add i8 %29, %27
  %31 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %9, i64 %24
  %32 = load i8, i8* %31, align 1, !tbaa !7
  %33 = add i8 %30, %32
  %34 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 %25
  %35 = load i8, i8* %34, align 1, !tbaa !7
  %36 = add i8 %33, %35
  %37 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 %24
  %38 = load i8, i8* %37, align 1, !tbaa !7
  %39 = add i8 %36, %38
  %40 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %25
  %41 = load i8, i8* %40, align 1, !tbaa !7
  %42 = add i8 %39, %41
  %43 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %16
  %44 = load i8, i8* %43, align 1, !tbaa !7
  %45 = add i8 %42, %44
  %46 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %24
  %47 = load i8, i8* %46, align 1, !tbaa !7
  %48 = add i8 %45, %47
  %49 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 %16
  store i8 %48, i8* %49, align 1, !tbaa !7
  %50 = icmp eq i64 %20, 200
  br i1 %50, label %13, label %15, !llvm.loop !14
}

; Function Attrs: nounwind uwtable
define dso_local void @init_map(%struct.map_t* nocapture noundef %0) local_unnamed_addr #0 {
  br label %2

2:                                                ; preds = %1, %51
  %3 = phi i64 [ 0, %1 ], [ %52, %51 ]
  br label %54

4:                                                ; preds = %51, %13
  %5 = phi i64 [ %9, %13 ], [ 0, %51 ]
  %6 = trunc i64 %5 to i32
  %7 = add i32 %6, 99
  %8 = urem i32 %7, 100
  %9 = add nuw nsw i64 %5, 1
  %10 = icmp eq i64 %9, 100
  %11 = zext i32 %8 to i64
  %12 = select i1 %10, i64 0, i64 %9
  br label %14

13:                                               ; preds = %14
  br i1 %10, label %50, label %4, !llvm.loop !13

14:                                               ; preds = %14, %4
  %15 = phi i64 [ 0, %4 ], [ %19, %14 ]
  %16 = trunc i64 %15 to i16
  %17 = add i16 %16, 199
  %18 = urem i16 %17, 200
  %19 = add nuw nsw i64 %15, 1
  %20 = icmp ult i64 %15, 199
  %21 = add nuw nsw i64 %15, 57
  %22 = select i1 %20, i64 %19, i64 %21
  %23 = and i64 %22, 255
  %24 = zext i16 %18 to i64
  %25 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %24
  %26 = load i8, i8* %25, align 1, !tbaa !7
  %27 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %15
  %28 = load i8, i8* %27, align 1, !tbaa !7
  %29 = add i8 %28, %26
  %30 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %11, i64 %23
  %31 = load i8, i8* %30, align 1, !tbaa !7
  %32 = add i8 %29, %31
  %33 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %5, i64 %24
  %34 = load i8, i8* %33, align 1, !tbaa !7
  %35 = add i8 %32, %34
  %36 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %5, i64 %23
  %37 = load i8, i8* %36, align 1, !tbaa !7
  %38 = add i8 %35, %37
  %39 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %12, i64 %24
  %40 = load i8, i8* %39, align 1, !tbaa !7
  %41 = add i8 %38, %40
  %42 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %12, i64 %15
  %43 = load i8, i8* %42, align 1, !tbaa !7
  %44 = add i8 %41, %43
  %45 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %12, i64 %23
  %46 = load i8, i8* %45, align 1, !tbaa !7
  %47 = add i8 %44, %46
  %48 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %5, i64 %15
  store i8 %47, i8* %48, align 1, !tbaa !7
  %49 = icmp eq i64 %19, 200
  br i1 %49, label %13, label %14, !llvm.loop !14

50:                                               ; preds = %13
  ret void

51:                                               ; preds = %54
  %52 = add nuw nsw i64 %3, 1
  %53 = icmp eq i64 %52, 100
  br i1 %53, label %4, label %2, !llvm.loop !15

54:                                               ; preds = %2, %54
  %55 = phi i64 [ 0, %2 ], [ %61, %54 ]
  %56 = tail call i32 (...) @sim_rand() #5
  %57 = srem i32 %56, 5
  %58 = icmp eq i32 %57, 0
  %59 = zext i1 %58 to i8
  %60 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 %55
  store i8 %59, i8* %60, align 1, !tbaa !7
  %61 = add nuw nsw i64 %55, 1
  %62 = icmp eq i64 %61, 200
  br i1 %62, label %51, label %54, !llvm.loop !16
}

declare i32 @sim_rand(...) local_unnamed_addr #2

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @update_map(%struct.map_t* nocapture noundef %0) local_unnamed_addr #3 {
  br label %2

2:                                                ; preds = %1, %2
  %3 = phi i64 [ 0, %1 ], [ %173, %2 ]
  %4 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 0
  %5 = bitcast i8* %4 to <16 x i8>*
  %6 = load <16 x i8>, <16 x i8>* %5, align 1, !tbaa !7
  %7 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 16
  %8 = bitcast i8* %7 to <16 x i8>*
  %9 = load <16 x i8>, <16 x i8>* %8, align 1, !tbaa !7
  %10 = icmp eq <16 x i8> %6, zeroinitializer
  %11 = icmp eq <16 x i8> %9, zeroinitializer
  %12 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 0
  %13 = bitcast i8* %12 to <16 x i8>*
  %14 = load <16 x i8>, <16 x i8>* %13, align 1, !tbaa !7
  %15 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 16
  %16 = bitcast i8* %15 to <16 x i8>*
  %17 = load <16 x i8>, <16 x i8>* %16, align 1, !tbaa !7
  %18 = and <16 x i8> %14, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %19 = and <16 x i8> %17, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %20 = icmp eq <16 x i8> %18, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %21 = icmp eq <16 x i8> %19, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %22 = icmp eq <16 x i8> %14, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %23 = icmp eq <16 x i8> %17, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %24 = select <16 x i1> %10, <16 x i1> %22, <16 x i1> %20
  %25 = select <16 x i1> %11, <16 x i1> %23, <16 x i1> %21
  %26 = zext <16 x i1> %24 to <16 x i8>
  %27 = zext <16 x i1> %25 to <16 x i8>
  %28 = bitcast i8* %4 to <16 x i8>*
  store <16 x i8> %26, <16 x i8>* %28, align 1, !tbaa !7
  %29 = bitcast i8* %7 to <16 x i8>*
  store <16 x i8> %27, <16 x i8>* %29, align 1, !tbaa !7
  %30 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 32
  %31 = bitcast i8* %30 to <16 x i8>*
  %32 = load <16 x i8>, <16 x i8>* %31, align 1, !tbaa !7
  %33 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 48
  %34 = bitcast i8* %33 to <16 x i8>*
  %35 = load <16 x i8>, <16 x i8>* %34, align 1, !tbaa !7
  %36 = icmp eq <16 x i8> %32, zeroinitializer
  %37 = icmp eq <16 x i8> %35, zeroinitializer
  %38 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 32
  %39 = bitcast i8* %38 to <16 x i8>*
  %40 = load <16 x i8>, <16 x i8>* %39, align 1, !tbaa !7
  %41 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 48
  %42 = bitcast i8* %41 to <16 x i8>*
  %43 = load <16 x i8>, <16 x i8>* %42, align 1, !tbaa !7
  %44 = and <16 x i8> %40, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %45 = and <16 x i8> %43, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %46 = icmp eq <16 x i8> %44, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %47 = icmp eq <16 x i8> %45, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %48 = icmp eq <16 x i8> %40, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %49 = icmp eq <16 x i8> %43, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %50 = select <16 x i1> %36, <16 x i1> %48, <16 x i1> %46
  %51 = select <16 x i1> %37, <16 x i1> %49, <16 x i1> %47
  %52 = zext <16 x i1> %50 to <16 x i8>
  %53 = zext <16 x i1> %51 to <16 x i8>
  %54 = bitcast i8* %30 to <16 x i8>*
  store <16 x i8> %52, <16 x i8>* %54, align 1, !tbaa !7
  %55 = bitcast i8* %33 to <16 x i8>*
  store <16 x i8> %53, <16 x i8>* %55, align 1, !tbaa !7
  %56 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 64
  %57 = bitcast i8* %56 to <16 x i8>*
  %58 = load <16 x i8>, <16 x i8>* %57, align 1, !tbaa !7
  %59 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 80
  %60 = bitcast i8* %59 to <16 x i8>*
  %61 = load <16 x i8>, <16 x i8>* %60, align 1, !tbaa !7
  %62 = icmp eq <16 x i8> %58, zeroinitializer
  %63 = icmp eq <16 x i8> %61, zeroinitializer
  %64 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 64
  %65 = bitcast i8* %64 to <16 x i8>*
  %66 = load <16 x i8>, <16 x i8>* %65, align 1, !tbaa !7
  %67 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 80
  %68 = bitcast i8* %67 to <16 x i8>*
  %69 = load <16 x i8>, <16 x i8>* %68, align 1, !tbaa !7
  %70 = and <16 x i8> %66, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %71 = and <16 x i8> %69, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %72 = icmp eq <16 x i8> %70, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %73 = icmp eq <16 x i8> %71, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %74 = icmp eq <16 x i8> %66, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %75 = icmp eq <16 x i8> %69, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %76 = select <16 x i1> %62, <16 x i1> %74, <16 x i1> %72
  %77 = select <16 x i1> %63, <16 x i1> %75, <16 x i1> %73
  %78 = zext <16 x i1> %76 to <16 x i8>
  %79 = zext <16 x i1> %77 to <16 x i8>
  %80 = bitcast i8* %56 to <16 x i8>*
  store <16 x i8> %78, <16 x i8>* %80, align 1, !tbaa !7
  %81 = bitcast i8* %59 to <16 x i8>*
  store <16 x i8> %79, <16 x i8>* %81, align 1, !tbaa !7
  %82 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 96
  %83 = bitcast i8* %82 to <16 x i8>*
  %84 = load <16 x i8>, <16 x i8>* %83, align 1, !tbaa !7
  %85 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 112
  %86 = bitcast i8* %85 to <16 x i8>*
  %87 = load <16 x i8>, <16 x i8>* %86, align 1, !tbaa !7
  %88 = icmp eq <16 x i8> %84, zeroinitializer
  %89 = icmp eq <16 x i8> %87, zeroinitializer
  %90 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 96
  %91 = bitcast i8* %90 to <16 x i8>*
  %92 = load <16 x i8>, <16 x i8>* %91, align 1, !tbaa !7
  %93 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 112
  %94 = bitcast i8* %93 to <16 x i8>*
  %95 = load <16 x i8>, <16 x i8>* %94, align 1, !tbaa !7
  %96 = and <16 x i8> %92, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %97 = and <16 x i8> %95, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %98 = icmp eq <16 x i8> %96, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %99 = icmp eq <16 x i8> %97, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %100 = icmp eq <16 x i8> %92, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %101 = icmp eq <16 x i8> %95, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %102 = select <16 x i1> %88, <16 x i1> %100, <16 x i1> %98
  %103 = select <16 x i1> %89, <16 x i1> %101, <16 x i1> %99
  %104 = zext <16 x i1> %102 to <16 x i8>
  %105 = zext <16 x i1> %103 to <16 x i8>
  %106 = bitcast i8* %82 to <16 x i8>*
  store <16 x i8> %104, <16 x i8>* %106, align 1, !tbaa !7
  %107 = bitcast i8* %85 to <16 x i8>*
  store <16 x i8> %105, <16 x i8>* %107, align 1, !tbaa !7
  %108 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 128
  %109 = bitcast i8* %108 to <16 x i8>*
  %110 = load <16 x i8>, <16 x i8>* %109, align 1, !tbaa !7
  %111 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 144
  %112 = bitcast i8* %111 to <16 x i8>*
  %113 = load <16 x i8>, <16 x i8>* %112, align 1, !tbaa !7
  %114 = icmp eq <16 x i8> %110, zeroinitializer
  %115 = icmp eq <16 x i8> %113, zeroinitializer
  %116 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 128
  %117 = bitcast i8* %116 to <16 x i8>*
  %118 = load <16 x i8>, <16 x i8>* %117, align 1, !tbaa !7
  %119 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 144
  %120 = bitcast i8* %119 to <16 x i8>*
  %121 = load <16 x i8>, <16 x i8>* %120, align 1, !tbaa !7
  %122 = and <16 x i8> %118, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %123 = and <16 x i8> %121, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %124 = icmp eq <16 x i8> %122, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %125 = icmp eq <16 x i8> %123, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %126 = icmp eq <16 x i8> %118, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %127 = icmp eq <16 x i8> %121, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %128 = select <16 x i1> %114, <16 x i1> %126, <16 x i1> %124
  %129 = select <16 x i1> %115, <16 x i1> %127, <16 x i1> %125
  %130 = zext <16 x i1> %128 to <16 x i8>
  %131 = zext <16 x i1> %129 to <16 x i8>
  %132 = bitcast i8* %108 to <16 x i8>*
  store <16 x i8> %130, <16 x i8>* %132, align 1, !tbaa !7
  %133 = bitcast i8* %111 to <16 x i8>*
  store <16 x i8> %131, <16 x i8>* %133, align 1, !tbaa !7
  %134 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 160
  %135 = bitcast i8* %134 to <16 x i8>*
  %136 = load <16 x i8>, <16 x i8>* %135, align 1, !tbaa !7
  %137 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 176
  %138 = bitcast i8* %137 to <16 x i8>*
  %139 = load <16 x i8>, <16 x i8>* %138, align 1, !tbaa !7
  %140 = icmp eq <16 x i8> %136, zeroinitializer
  %141 = icmp eq <16 x i8> %139, zeroinitializer
  %142 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 160
  %143 = bitcast i8* %142 to <16 x i8>*
  %144 = load <16 x i8>, <16 x i8>* %143, align 1, !tbaa !7
  %145 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 176
  %146 = bitcast i8* %145 to <16 x i8>*
  %147 = load <16 x i8>, <16 x i8>* %146, align 1, !tbaa !7
  %148 = and <16 x i8> %144, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %149 = and <16 x i8> %147, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %150 = icmp eq <16 x i8> %148, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %151 = icmp eq <16 x i8> %149, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %152 = icmp eq <16 x i8> %144, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %153 = icmp eq <16 x i8> %147, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %154 = select <16 x i1> %140, <16 x i1> %152, <16 x i1> %150
  %155 = select <16 x i1> %141, <16 x i1> %153, <16 x i1> %151
  %156 = zext <16 x i1> %154 to <16 x i8>
  %157 = zext <16 x i1> %155 to <16 x i8>
  %158 = bitcast i8* %134 to <16 x i8>*
  store <16 x i8> %156, <16 x i8>* %158, align 1, !tbaa !7
  %159 = bitcast i8* %137 to <16 x i8>*
  store <16 x i8> %157, <16 x i8>* %159, align 1, !tbaa !7
  %160 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %3, i64 192
  %161 = bitcast i8* %160 to <8 x i8>*
  %162 = load <8 x i8>, <8 x i8>* %161, align 1, !tbaa !7
  %163 = icmp eq <8 x i8> %162, zeroinitializer
  %164 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %3, i64 192
  %165 = bitcast i8* %164 to <8 x i8>*
  %166 = load <8 x i8>, <8 x i8>* %165, align 1, !tbaa !7
  %167 = and <8 x i8> %166, <i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2, i8 -2>
  %168 = icmp eq <8 x i8> %167, <i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2, i8 2>
  %169 = icmp eq <8 x i8> %166, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  %170 = select <8 x i1> %163, <8 x i1> %169, <8 x i1> %168
  %171 = zext <8 x i1> %170 to <8 x i8>
  %172 = bitcast i8* %160 to <8 x i8>*
  store <8 x i8> %171, <8 x i8>* %172, align 1, !tbaa !7
  %173 = add nuw nsw i64 %3, 1
  %174 = icmp eq i64 %173, 100
  br i1 %174, label %175, label %2, !llvm.loop !17

175:                                              ; preds = %2, %184
  %176 = phi i64 [ %180, %184 ], [ 0, %2 ]
  %177 = trunc i64 %176 to i32
  %178 = add i32 %177, 99
  %179 = urem i32 %178, 100
  %180 = add nuw nsw i64 %176, 1
  %181 = icmp eq i64 %180, 100
  %182 = zext i32 %179 to i64
  %183 = select i1 %181, i64 0, i64 %180
  br label %185

184:                                              ; preds = %185
  br i1 %181, label %221, label %175, !llvm.loop !13

185:                                              ; preds = %185, %175
  %186 = phi i64 [ 0, %175 ], [ %190, %185 ]
  %187 = trunc i64 %186 to i16
  %188 = add i16 %187, 199
  %189 = urem i16 %188, 200
  %190 = add nuw nsw i64 %186, 1
  %191 = icmp ult i64 %186, 199
  %192 = add nuw nsw i64 %186, 57
  %193 = select i1 %191, i64 %190, i64 %192
  %194 = and i64 %193, 255
  %195 = zext i16 %189 to i64
  %196 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %195
  %197 = load i8, i8* %196, align 1, !tbaa !7
  %198 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %186
  %199 = load i8, i8* %198, align 1, !tbaa !7
  %200 = add i8 %199, %197
  %201 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %182, i64 %194
  %202 = load i8, i8* %201, align 1, !tbaa !7
  %203 = add i8 %200, %202
  %204 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %195
  %205 = load i8, i8* %204, align 1, !tbaa !7
  %206 = add i8 %203, %205
  %207 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %176, i64 %194
  %208 = load i8, i8* %207, align 1, !tbaa !7
  %209 = add i8 %206, %208
  %210 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %195
  %211 = load i8, i8* %210, align 1, !tbaa !7
  %212 = add i8 %209, %211
  %213 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %186
  %214 = load i8, i8* %213, align 1, !tbaa !7
  %215 = add i8 %212, %214
  %216 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 0, i64 %183, i64 %194
  %217 = load i8, i8* %216, align 1, !tbaa !7
  %218 = add i8 %215, %217
  %219 = getelementptr inbounds %struct.map_t, %struct.map_t* %0, i64 0, i32 1, i64 %176, i64 %186
  store i8 %218, i8* %219, align 1, !tbaa !7
  %220 = icmp eq i64 %190, 200
  br i1 %220, label %184, label %185, !llvm.loop !14

221:                                              ; preds = %184
  ret void
}

; Function Attrs: noreturn nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #4 {
  %1 = alloca %struct.map_t, align 1
  %2 = getelementptr inbounds %struct.map_t, %struct.map_t* %1, i64 0, i32 0, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 40000, i8* nonnull %2) #5
  call void @init_map(%struct.map_t* noundef nonnull %1)
  br label %3

3:                                                ; preds = %25, %0
  tail call void @sim_clear(i32 noundef -2139062017) #5
  br label %4

4:                                                ; preds = %7, %3
  %5 = phi i64 [ 0, %3 ], [ %8, %7 ]
  %6 = trunc i64 %5 to i32
  br label %10

7:                                                ; preds = %22
  %8 = add nuw nsw i64 %5, 1
  %9 = icmp eq i64 %8, 100
  br i1 %9, label %25, label %4, !llvm.loop !5

10:                                               ; preds = %22, %4
  %11 = phi i64 [ 0, %4 ], [ %23, %22 ]
  %12 = getelementptr inbounds %struct.map_t, %struct.map_t* %1, i64 0, i32 0, i64 %5, i64 %11
  %13 = load i8, i8* %12, align 1, !tbaa !7
  %14 = icmp eq i8 %13, 0
  br i1 %14, label %22, label %15

15:                                               ; preds = %10
  %16 = getelementptr inbounds %struct.map_t, %struct.map_t* %1, i64 0, i32 1, i64 %5, i64 %11
  %17 = load i8, i8* %16, align 1, !tbaa !7
  %18 = zext i8 %17 to i64
  %19 = getelementptr inbounds [9 x i32], [9 x i32]* @__const.draw_map.color_scheme, i64 0, i64 %18
  %20 = load i32, i32* %19, align 4, !tbaa !10
  %21 = trunc i64 %11 to i32
  tail call void @sim_set_pixel(i32 noundef %21, i32 noundef %6, i32 noundef %20, i32 noundef 32) #5
  br label %22

22:                                               ; preds = %15, %10
  %23 = add nuw nsw i64 %11, 1
  %24 = icmp eq i64 %23, 200
  br i1 %24, label %7, label %10, !llvm.loop !12

25:                                               ; preds = %7
  tail call void (...) @sim_display() #5
  call void @update_map(%struct.map_t* noundef nonnull %1)
  br label %3
}

declare void @sim_clear(i32 noundef) local_unnamed_addr #2

declare void @sim_display(...) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree norecurse nosync nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noreturn nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"Ubuntu clang version 14.0.0-1ubuntu1.1"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !8, i64 0}
!12 = distinct !{!12, !6}
!13 = distinct !{!13, !6}
!14 = distinct !{!14, !6}
!15 = distinct !{!15, !6}
!16 = distinct !{!16, !6}
!17 = distinct !{!17, !6}
