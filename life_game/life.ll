; ModuleID = '../src/life.c'
source_filename = "../src/life.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx12.0.0"

%struct.map_t = type { [200 x [100 x i8]], [200 x [100 x i8]] }

@__const.draw_map.color_scheme = private unnamed_addr constant [9 x i32] [i32 -253811457, i32 1659905279, i32 619763455, i32 616755455, i32 607318271, i32 -1024659201, i32 -266068737, i32 -264493825, i32 -260430593], align 16

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @draw_map(%struct.map_t* %0) #0 {
  %2 = alloca %struct.map_t*, align 8
  %3 = alloca [9 x i32], align 16
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store %struct.map_t* %0, %struct.map_t** %2, align 8
  %6 = bitcast [9 x i32]* %3 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %6, i8* align 16 bitcast ([9 x i32]* @__const.draw_map.color_scheme to i8*), i64 36, i1 false)
  store i32 0, i32* %4, align 4
  br label %7

7:                                                ; preds = %45, %1
  %8 = load i32, i32* %4, align 4
  %9 = icmp slt i32 %8, 100
  br i1 %9, label %10, label %48

10:                                               ; preds = %7
  store i32 0, i32* %5, align 4
  br label %11

11:                                               ; preds = %41, %10
  %12 = load i32, i32* %5, align 4
  %13 = icmp slt i32 %12, 200
  br i1 %13, label %14, label %44

14:                                               ; preds = %11
  %15 = load %struct.map_t*, %struct.map_t** %2, align 8
  %16 = getelementptr inbounds %struct.map_t, %struct.map_t* %15, i32 0, i32 0
  %17 = load i32, i32* %4, align 4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %16, i64 0, i64 %18
  %20 = load i32, i32* %5, align 4
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds [100 x i8], [100 x i8]* %19, i64 0, i64 %21
  %23 = load i8, i8* %22, align 1
  %24 = icmp ne i8 %23, 0
  br i1 %24, label %25, label %40

25:                                               ; preds = %14
  %26 = load i32, i32* %5, align 4
  %27 = load i32, i32* %4, align 4
  %28 = load %struct.map_t*, %struct.map_t** %2, align 8
  %29 = getelementptr inbounds %struct.map_t, %struct.map_t* %28, i32 0, i32 1
  %30 = load i32, i32* %4, align 4
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %29, i64 0, i64 %31
  %33 = load i32, i32* %5, align 4
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds [100 x i8], [100 x i8]* %32, i64 0, i64 %34
  %36 = load i8, i8* %35, align 1
  %37 = zext i8 %36 to i64
  %38 = getelementptr inbounds [9 x i32], [9 x i32]* %3, i64 0, i64 %37
  %39 = load i32, i32* %38, align 4
  call void @sim_set_pixel(i32 %26, i32 %27, i32 %39, i32 32)
  br label %40

40:                                               ; preds = %25, %14
  br label %41

41:                                               ; preds = %40
  %42 = load i32, i32* %5, align 4
  %43 = add nsw i32 %42, 1
  store i32 %43, i32* %5, align 4
  br label %11, !llvm.loop !5

44:                                               ; preds = %11
  br label %45

45:                                               ; preds = %44
  %46 = load i32, i32* %4, align 4
  %47 = add nsw i32 %46, 1
  store i32 %47, i32* %4, align 4
  br label %7, !llvm.loop !7

48:                                               ; preds = %7
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

declare void @sim_set_pixel(i32, i32, i32, i32) #2

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @update_neighbour_count(%struct.map_t* %0) #0 {
  %2 = alloca %struct.map_t*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store %struct.map_t* %0, %struct.map_t** %2, align 8
  store i32 0, i32* %3, align 4
  br label %9

9:                                                ; preds = %131, %1
  %10 = load i32, i32* %3, align 4
  %11 = icmp slt i32 %10, 100
  br i1 %11, label %12, label %134

12:                                               ; preds = %9
  store i32 0, i32* %4, align 4
  br label %13

13:                                               ; preds = %127, %12
  %14 = load i32, i32* %4, align 4
  %15 = icmp slt i32 %14, 200
  br i1 %15, label %16, label %130

16:                                               ; preds = %13
  %17 = load i32, i32* %4, align 4
  %18 = add nsw i32 %17, 200
  %19 = sub nsw i32 %18, 1
  %20 = srem i32 %19, 200
  store i32 %20, i32* %5, align 4
  %21 = load i32, i32* %4, align 4
  %22 = add nsw i32 %21, 1
  %23 = srem i32 %22, 200
  store i32 %23, i32* %6, align 4
  %24 = load i32, i32* %3, align 4
  %25 = add nsw i32 %24, 100
  %26 = sub nsw i32 %25, 1
  %27 = srem i32 %26, 100
  store i32 %27, i32* %7, align 4
  %28 = load i32, i32* %3, align 4
  %29 = add nsw i32 %28, 1
  %30 = srem i32 %29, 100
  store i32 %30, i32* %8, align 4
  %31 = load %struct.map_t*, %struct.map_t** %2, align 8
  %32 = getelementptr inbounds %struct.map_t, %struct.map_t* %31, i32 0, i32 0
  %33 = load i32, i32* %7, align 4
  %34 = zext i32 %33 to i64
  %35 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %32, i64 0, i64 %34
  %36 = load i32, i32* %5, align 4
  %37 = zext i32 %36 to i64
  %38 = getelementptr inbounds [100 x i8], [100 x i8]* %35, i64 0, i64 %37
  %39 = load i8, i8* %38, align 1
  %40 = zext i8 %39 to i32
  %41 = load %struct.map_t*, %struct.map_t** %2, align 8
  %42 = getelementptr inbounds %struct.map_t, %struct.map_t* %41, i32 0, i32 0
  %43 = load i32, i32* %7, align 4
  %44 = zext i32 %43 to i64
  %45 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %42, i64 0, i64 %44
  %46 = load i32, i32* %4, align 4
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds [100 x i8], [100 x i8]* %45, i64 0, i64 %47
  %49 = load i8, i8* %48, align 1
  %50 = zext i8 %49 to i32
  %51 = add nsw i32 %40, %50
  %52 = load %struct.map_t*, %struct.map_t** %2, align 8
  %53 = getelementptr inbounds %struct.map_t, %struct.map_t* %52, i32 0, i32 0
  %54 = load i32, i32* %7, align 4
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %53, i64 0, i64 %55
  %57 = load i32, i32* %6, align 4
  %58 = zext i32 %57 to i64
  %59 = getelementptr inbounds [100 x i8], [100 x i8]* %56, i64 0, i64 %58
  %60 = load i8, i8* %59, align 1
  %61 = zext i8 %60 to i32
  %62 = add nsw i32 %51, %61
  %63 = load %struct.map_t*, %struct.map_t** %2, align 8
  %64 = getelementptr inbounds %struct.map_t, %struct.map_t* %63, i32 0, i32 0
  %65 = load i32, i32* %3, align 4
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %64, i64 0, i64 %66
  %68 = load i32, i32* %5, align 4
  %69 = zext i32 %68 to i64
  %70 = getelementptr inbounds [100 x i8], [100 x i8]* %67, i64 0, i64 %69
  %71 = load i8, i8* %70, align 1
  %72 = zext i8 %71 to i32
  %73 = add nsw i32 %62, %72
  %74 = load %struct.map_t*, %struct.map_t** %2, align 8
  %75 = getelementptr inbounds %struct.map_t, %struct.map_t* %74, i32 0, i32 0
  %76 = load i32, i32* %3, align 4
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %75, i64 0, i64 %77
  %79 = load i32, i32* %6, align 4
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds [100 x i8], [100 x i8]* %78, i64 0, i64 %80
  %82 = load i8, i8* %81, align 1
  %83 = zext i8 %82 to i32
  %84 = add nsw i32 %73, %83
  %85 = load %struct.map_t*, %struct.map_t** %2, align 8
  %86 = getelementptr inbounds %struct.map_t, %struct.map_t* %85, i32 0, i32 0
  %87 = load i32, i32* %8, align 4
  %88 = zext i32 %87 to i64
  %89 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %86, i64 0, i64 %88
  %90 = load i32, i32* %5, align 4
  %91 = zext i32 %90 to i64
  %92 = getelementptr inbounds [100 x i8], [100 x i8]* %89, i64 0, i64 %91
  %93 = load i8, i8* %92, align 1
  %94 = zext i8 %93 to i32
  %95 = add nsw i32 %84, %94
  %96 = load %struct.map_t*, %struct.map_t** %2, align 8
  %97 = getelementptr inbounds %struct.map_t, %struct.map_t* %96, i32 0, i32 0
  %98 = load i32, i32* %8, align 4
  %99 = zext i32 %98 to i64
  %100 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %97, i64 0, i64 %99
  %101 = load i32, i32* %4, align 4
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds [100 x i8], [100 x i8]* %100, i64 0, i64 %102
  %104 = load i8, i8* %103, align 1
  %105 = zext i8 %104 to i32
  %106 = add nsw i32 %95, %105
  %107 = load %struct.map_t*, %struct.map_t** %2, align 8
  %108 = getelementptr inbounds %struct.map_t, %struct.map_t* %107, i32 0, i32 0
  %109 = load i32, i32* %8, align 4
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %108, i64 0, i64 %110
  %112 = load i32, i32* %6, align 4
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds [100 x i8], [100 x i8]* %111, i64 0, i64 %113
  %115 = load i8, i8* %114, align 1
  %116 = zext i8 %115 to i32
  %117 = add nsw i32 %106, %116
  %118 = trunc i32 %117 to i8
  %119 = load %struct.map_t*, %struct.map_t** %2, align 8
  %120 = getelementptr inbounds %struct.map_t, %struct.map_t* %119, i32 0, i32 1
  %121 = load i32, i32* %3, align 4
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %120, i64 0, i64 %122
  %124 = load i32, i32* %4, align 4
  %125 = sext i32 %124 to i64
  %126 = getelementptr inbounds [100 x i8], [100 x i8]* %123, i64 0, i64 %125
  store i8 %118, i8* %126, align 1
  br label %127

127:                                              ; preds = %16
  %128 = load i32, i32* %4, align 4
  %129 = add nsw i32 %128, 1
  store i32 %129, i32* %4, align 4
  br label %13, !llvm.loop !8

130:                                              ; preds = %13
  br label %131

131:                                              ; preds = %130
  %132 = load i32, i32* %3, align 4
  %133 = add nsw i32 %132, 1
  store i32 %133, i32* %3, align 4
  br label %9, !llvm.loop !9

134:                                              ; preds = %9
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @init_map(%struct.map_t* %0) #0 {
  %2 = alloca %struct.map_t*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %struct.map_t* %0, %struct.map_t** %2, align 8
  store i32 0, i32* %3, align 4
  br label %5

5:                                                ; preds = %30, %1
  %6 = load i32, i32* %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %33

8:                                                ; preds = %5
  store i32 0, i32* %4, align 4
  br label %9

9:                                                ; preds = %26, %8
  %10 = load i32, i32* %4, align 4
  %11 = icmp slt i32 %10, 200
  br i1 %11, label %12, label %29

12:                                               ; preds = %9
  %13 = call i32 (...) @sim_rand()
  %14 = srem i32 %13, 5
  %15 = icmp eq i32 %14, 0
  %16 = zext i1 %15 to i32
  %17 = trunc i32 %16 to i8
  %18 = load %struct.map_t*, %struct.map_t** %2, align 8
  %19 = getelementptr inbounds %struct.map_t, %struct.map_t* %18, i32 0, i32 0
  %20 = load i32, i32* %3, align 4
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %19, i64 0, i64 %21
  %23 = load i32, i32* %4, align 4
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds [100 x i8], [100 x i8]* %22, i64 0, i64 %24
  store i8 %17, i8* %25, align 1
  br label %26

26:                                               ; preds = %12
  %27 = load i32, i32* %4, align 4
  %28 = add nsw i32 %27, 1
  store i32 %28, i32* %4, align 4
  br label %9, !llvm.loop !10

29:                                               ; preds = %9
  br label %30

30:                                               ; preds = %29
  %31 = load i32, i32* %3, align 4
  %32 = add nsw i32 %31, 1
  store i32 %32, i32* %3, align 4
  br label %5, !llvm.loop !11

33:                                               ; preds = %5
  %34 = load %struct.map_t*, %struct.map_t** %2, align 8
  call void @update_neighbour_count(%struct.map_t* %34)
  ret void
}

declare i32 @sim_rand(...) #2

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @update_map(%struct.map_t* %0) #0 {
  %2 = alloca %struct.map_t*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %struct.map_t* %0, %struct.map_t** %2, align 8
  store i32 0, i32* %3, align 4
  br label %5

5:                                                ; preds = %101, %1
  %6 = load i32, i32* %3, align 4
  %7 = icmp slt i32 %6, 100
  br i1 %7, label %8, label %104

8:                                                ; preds = %5
  store i32 0, i32* %4, align 4
  br label %9

9:                                                ; preds = %97, %8
  %10 = load i32, i32* %4, align 4
  %11 = icmp slt i32 %10, 200
  br i1 %11, label %12, label %100

12:                                               ; preds = %9
  %13 = load %struct.map_t*, %struct.map_t** %2, align 8
  %14 = getelementptr inbounds %struct.map_t, %struct.map_t* %13, i32 0, i32 0
  %15 = load i32, i32* %3, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %14, i64 0, i64 %16
  %18 = load i32, i32* %4, align 4
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds [100 x i8], [100 x i8]* %17, i64 0, i64 %19
  %21 = load i8, i8* %20, align 1
  %22 = zext i8 %21 to i32
  %23 = icmp ne i32 %22, 0
  br i1 %23, label %24, label %36

24:                                               ; preds = %12
  %25 = load %struct.map_t*, %struct.map_t** %2, align 8
  %26 = getelementptr inbounds %struct.map_t, %struct.map_t* %25, i32 0, i32 1
  %27 = load i32, i32* %3, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %26, i64 0, i64 %28
  %30 = load i32, i32* %4, align 4
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds [100 x i8], [100 x i8]* %29, i64 0, i64 %31
  %33 = load i8, i8* %32, align 1
  %34 = zext i8 %33 to i32
  %35 = icmp eq i32 %34, 2
  br i1 %35, label %85, label %36

36:                                               ; preds = %24, %12
  %37 = load %struct.map_t*, %struct.map_t** %2, align 8
  %38 = getelementptr inbounds %struct.map_t, %struct.map_t* %37, i32 0, i32 0
  %39 = load i32, i32* %3, align 4
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %38, i64 0, i64 %40
  %42 = load i32, i32* %4, align 4
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds [100 x i8], [100 x i8]* %41, i64 0, i64 %43
  %45 = load i8, i8* %44, align 1
  %46 = zext i8 %45 to i32
  %47 = icmp ne i32 %46, 0
  br i1 %47, label %48, label %60

48:                                               ; preds = %36
  %49 = load %struct.map_t*, %struct.map_t** %2, align 8
  %50 = getelementptr inbounds %struct.map_t, %struct.map_t* %49, i32 0, i32 1
  %51 = load i32, i32* %3, align 4
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %50, i64 0, i64 %52
  %54 = load i32, i32* %4, align 4
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds [100 x i8], [100 x i8]* %53, i64 0, i64 %55
  %57 = load i8, i8* %56, align 1
  %58 = zext i8 %57 to i32
  %59 = icmp eq i32 %58, 3
  br i1 %59, label %85, label %60

60:                                               ; preds = %48, %36
  %61 = load %struct.map_t*, %struct.map_t** %2, align 8
  %62 = getelementptr inbounds %struct.map_t, %struct.map_t* %61, i32 0, i32 0
  %63 = load i32, i32* %3, align 4
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %62, i64 0, i64 %64
  %66 = load i32, i32* %4, align 4
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds [100 x i8], [100 x i8]* %65, i64 0, i64 %67
  %69 = load i8, i8* %68, align 1
  %70 = icmp ne i8 %69, 0
  br i1 %70, label %83, label %71

71:                                               ; preds = %60
  %72 = load %struct.map_t*, %struct.map_t** %2, align 8
  %73 = getelementptr inbounds %struct.map_t, %struct.map_t* %72, i32 0, i32 1
  %74 = load i32, i32* %3, align 4
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %73, i64 0, i64 %75
  %77 = load i32, i32* %4, align 4
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds [100 x i8], [100 x i8]* %76, i64 0, i64 %78
  %80 = load i8, i8* %79, align 1
  %81 = zext i8 %80 to i32
  %82 = icmp eq i32 %81, 3
  br label %83

83:                                               ; preds = %71, %60
  %84 = phi i1 [ false, %60 ], [ %82, %71 ]
  br label %85

85:                                               ; preds = %83, %48, %24
  %86 = phi i1 [ true, %48 ], [ true, %24 ], [ %84, %83 ]
  %87 = zext i1 %86 to i32
  %88 = trunc i32 %87 to i8
  %89 = load %struct.map_t*, %struct.map_t** %2, align 8
  %90 = getelementptr inbounds %struct.map_t, %struct.map_t* %89, i32 0, i32 0
  %91 = load i32, i32* %3, align 4
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* %90, i64 0, i64 %92
  %94 = load i32, i32* %4, align 4
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds [100 x i8], [100 x i8]* %93, i64 0, i64 %95
  store i8 %88, i8* %96, align 1
  br label %97

97:                                               ; preds = %85
  %98 = load i32, i32* %4, align 4
  %99 = add nsw i32 %98, 1
  store i32 %99, i32* %4, align 4
  br label %9, !llvm.loop !12

100:                                              ; preds = %9
  br label %101

101:                                              ; preds = %100
  %102 = load i32, i32* %3, align 4
  %103 = add nsw i32 %102, 1
  store i32 %103, i32* %3, align 4
  br label %5, !llvm.loop !13

104:                                              ; preds = %5
  %105 = load %struct.map_t*, %struct.map_t** %2, align 8
  call void @update_neighbour_count(%struct.map_t* %105)
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.map_t, align 1
  store i32 0, i32* %1, align 4
  call void @init_map(%struct.map_t* %2)
  br label %3

3:                                                ; preds = %0, %3
  call void @sim_clear(i32 -2139062017)
  call void @draw_map(%struct.map_t* %2)
  call void (...) @sim_display()
  call void @update_map(%struct.map_t* %2)
  br label %3
}

declare void @sim_clear(i32) #2

declare void @sim_display(...) #2

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 14.0.0 (https://github.com/llvm/llvm-project.git 2f16b87b4b1d3fee07aff71a9bf79c3a8d514e7a)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !6}
!10 = distinct !{!10, !6}
!11 = distinct !{!11, !6}
!12 = distinct !{!12, !6}
!13 = distinct !{!13, !6}
