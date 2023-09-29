; ModuleID = 'src/life.c'
source_filename = "src/life.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx12.0.0"

@color_scheme = global [9 x i32] [i32 -253811457, i32 1659905279, i32 619763455, i32 616755455, i32 607318271, i32 -1024659201, i32 -266068737, i32 -264493825, i32 -260430593], align 16
@rand_int.seed = internal global i32 123456789, align 4
@map = global [200 x [100 x i8]] zeroinitializer, align 16
@neighbour_count = global [200 x [100 x i8]] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @rand_int() #0 {
  %1 = load i32, i32* @rand_int.seed, align 4
  %2 = mul nsw i32 1103515245, %1
  %3 = add nsw i32 %2, 12345
  %4 = srem i32 %3, -2147483648
  store i32 %4, i32* @rand_int.seed, align 4
  ret i32 %4
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @draw_map() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %3

3:                                                ; preds = %37, %0
  %4 = load i32, i32* %1, align 4
  %5 = icmp slt i32 %4, 100
  br i1 %5, label %6, label %40

6:                                                ; preds = %3
  store i32 0, i32* %2, align 4
  br label %7

7:                                                ; preds = %33, %6
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 200
  br i1 %9, label %10, label %36

10:                                               ; preds = %7
  %11 = load i32, i32* %1, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %12
  %14 = load i32, i32* %2, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds [100 x i8], [100 x i8]* %13, i64 0, i64 %15
  %17 = load i8, i8* %16, align 1
  %18 = icmp ne i8 %17, 0
  br i1 %18, label %19, label %32

19:                                               ; preds = %10
  %20 = load i32, i32* %2, align 4
  %21 = load i32, i32* %1, align 4
  %22 = load i32, i32* %1, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @neighbour_count, i64 0, i64 %23
  %25 = load i32, i32* %2, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds [100 x i8], [100 x i8]* %24, i64 0, i64 %26
  %28 = load i8, i8* %27, align 1
  %29 = zext i8 %28 to i64
  %30 = getelementptr inbounds [9 x i32], [9 x i32]* @color_scheme, i64 0, i64 %29
  %31 = load i32, i32* %30, align 4
  call void @sim_set_pixel(i32 %20, i32 %21, i32 %31, i32 32)
  br label %32

32:                                               ; preds = %19, %10
  br label %33

33:                                               ; preds = %32
  %34 = load i32, i32* %2, align 4
  %35 = add nsw i32 %34, 1
  store i32 %35, i32* %2, align 4
  br label %7, !llvm.loop !5

36:                                               ; preds = %7
  br label %37

37:                                               ; preds = %36
  %38 = load i32, i32* %1, align 4
  %39 = add nsw i32 %38, 1
  store i32 %39, i32* %1, align 4
  br label %3, !llvm.loop !7

40:                                               ; preds = %3
  ret void
}

declare void @sim_set_pixel(i32, i32, i32, i32) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @update_neighbour_count() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %7

7:                                                ; preds = %111, %0
  %8 = load i32, i32* %1, align 4
  %9 = icmp slt i32 %8, 100
  br i1 %9, label %10, label %114

10:                                               ; preds = %7
  store i32 0, i32* %2, align 4
  br label %11

11:                                               ; preds = %107, %10
  %12 = load i32, i32* %2, align 4
  %13 = icmp slt i32 %12, 200
  br i1 %13, label %14, label %110

14:                                               ; preds = %11
  %15 = load i32, i32* %2, align 4
  %16 = add nsw i32 %15, 200
  %17 = sub nsw i32 %16, 1
  %18 = srem i32 %17, 200
  store i32 %18, i32* %3, align 4
  %19 = load i32, i32* %2, align 4
  %20 = add nsw i32 %19, 1
  %21 = srem i32 %20, 200
  store i32 %21, i32* %4, align 4
  %22 = load i32, i32* %1, align 4
  %23 = add nsw i32 %22, 100
  %24 = sub nsw i32 %23, 1
  %25 = srem i32 %24, 100
  store i32 %25, i32* %5, align 4
  %26 = load i32, i32* %1, align 4
  %27 = add nsw i32 %26, 1
  %28 = srem i32 %27, 100
  store i32 %28, i32* %6, align 4
  %29 = load i32, i32* %5, align 4
  %30 = zext i32 %29 to i64
  %31 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %30
  %32 = load i32, i32* %3, align 4
  %33 = zext i32 %32 to i64
  %34 = getelementptr inbounds [100 x i8], [100 x i8]* %31, i64 0, i64 %33
  %35 = load i8, i8* %34, align 1
  %36 = zext i8 %35 to i32
  %37 = load i32, i32* %5, align 4
  %38 = zext i32 %37 to i64
  %39 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %38
  %40 = load i32, i32* %2, align 4
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds [100 x i8], [100 x i8]* %39, i64 0, i64 %41
  %43 = load i8, i8* %42, align 1
  %44 = zext i8 %43 to i32
  %45 = add nsw i32 %36, %44
  %46 = load i32, i32* %5, align 4
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %47
  %49 = load i32, i32* %4, align 4
  %50 = zext i32 %49 to i64
  %51 = getelementptr inbounds [100 x i8], [100 x i8]* %48, i64 0, i64 %50
  %52 = load i8, i8* %51, align 1
  %53 = zext i8 %52 to i32
  %54 = add nsw i32 %45, %53
  %55 = load i32, i32* %1, align 4
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %56
  %58 = load i32, i32* %3, align 4
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds [100 x i8], [100 x i8]* %57, i64 0, i64 %59
  %61 = load i8, i8* %60, align 1
  %62 = zext i8 %61 to i32
  %63 = add nsw i32 %54, %62
  %64 = load i32, i32* %1, align 4
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %65
  %67 = load i32, i32* %4, align 4
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds [100 x i8], [100 x i8]* %66, i64 0, i64 %68
  %70 = load i8, i8* %69, align 1
  %71 = zext i8 %70 to i32
  %72 = add nsw i32 %63, %71
  %73 = load i32, i32* %6, align 4
  %74 = zext i32 %73 to i64
  %75 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %74
  %76 = load i32, i32* %3, align 4
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds [100 x i8], [100 x i8]* %75, i64 0, i64 %77
  %79 = load i8, i8* %78, align 1
  %80 = zext i8 %79 to i32
  %81 = add nsw i32 %72, %80
  %82 = load i32, i32* %6, align 4
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %83
  %85 = load i32, i32* %2, align 4
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds [100 x i8], [100 x i8]* %84, i64 0, i64 %86
  %88 = load i8, i8* %87, align 1
  %89 = zext i8 %88 to i32
  %90 = add nsw i32 %81, %89
  %91 = load i32, i32* %6, align 4
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %92
  %94 = load i32, i32* %4, align 4
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds [100 x i8], [100 x i8]* %93, i64 0, i64 %95
  %97 = load i8, i8* %96, align 1
  %98 = zext i8 %97 to i32
  %99 = add nsw i32 %90, %98
  %100 = trunc i32 %99 to i8
  %101 = load i32, i32* %1, align 4
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @neighbour_count, i64 0, i64 %102
  %104 = load i32, i32* %2, align 4
  %105 = sext i32 %104 to i64
  %106 = getelementptr inbounds [100 x i8], [100 x i8]* %103, i64 0, i64 %105
  store i8 %100, i8* %106, align 1
  br label %107

107:                                              ; preds = %14
  %108 = load i32, i32* %2, align 4
  %109 = add nsw i32 %108, 1
  store i32 %109, i32* %2, align 4
  br label %11, !llvm.loop !8

110:                                              ; preds = %11
  br label %111

111:                                              ; preds = %110
  %112 = load i32, i32* %1, align 4
  %113 = add nsw i32 %112, 1
  store i32 %113, i32* %1, align 4
  br label %7, !llvm.loop !9

114:                                              ; preds = %7
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @init_map() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %3

3:                                                ; preds = %26, %0
  %4 = load i32, i32* %1, align 4
  %5 = icmp slt i32 %4, 100
  br i1 %5, label %6, label %29

6:                                                ; preds = %3
  store i32 0, i32* %2, align 4
  br label %7

7:                                                ; preds = %22, %6
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 200
  br i1 %9, label %10, label %25

10:                                               ; preds = %7
  %11 = call i32 @rand_int()
  %12 = srem i32 %11, 5
  %13 = icmp eq i32 %12, 0
  %14 = zext i1 %13 to i32
  %15 = trunc i32 %14 to i8
  %16 = load i32, i32* %1, align 4
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %17
  %19 = load i32, i32* %2, align 4
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [100 x i8], [100 x i8]* %18, i64 0, i64 %20
  store i8 %15, i8* %21, align 1
  br label %22

22:                                               ; preds = %10
  %23 = load i32, i32* %2, align 4
  %24 = add nsw i32 %23, 1
  store i32 %24, i32* %2, align 4
  br label %7, !llvm.loop !10

25:                                               ; preds = %7
  br label %26

26:                                               ; preds = %25
  %27 = load i32, i32* %1, align 4
  %28 = add nsw i32 %27, 1
  store i32 %28, i32* %1, align 4
  br label %3, !llvm.loop !11

29:                                               ; preds = %3
  call void @update_neighbour_count()
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @update_map() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %3

3:                                                ; preds = %85, %0
  %4 = load i32, i32* %1, align 4
  %5 = icmp slt i32 %4, 100
  br i1 %5, label %6, label %88

6:                                                ; preds = %3
  store i32 0, i32* %2, align 4
  br label %7

7:                                                ; preds = %81, %6
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 200
  br i1 %9, label %10, label %84

10:                                               ; preds = %7
  %11 = load i32, i32* %1, align 4
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %12
  %14 = load i32, i32* %2, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds [100 x i8], [100 x i8]* %13, i64 0, i64 %15
  %17 = load i8, i8* %16, align 1
  %18 = zext i8 %17 to i32
  %19 = icmp ne i32 %18, 0
  br i1 %19, label %20, label %30

20:                                               ; preds = %10
  %21 = load i32, i32* %1, align 4
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @neighbour_count, i64 0, i64 %22
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds [100 x i8], [100 x i8]* %23, i64 0, i64 %25
  %27 = load i8, i8* %26, align 1
  %28 = zext i8 %27 to i32
  %29 = icmp eq i32 %28, 2
  br i1 %29, label %71, label %30

30:                                               ; preds = %20, %10
  %31 = load i32, i32* %1, align 4
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %32
  %34 = load i32, i32* %2, align 4
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds [100 x i8], [100 x i8]* %33, i64 0, i64 %35
  %37 = load i8, i8* %36, align 1
  %38 = zext i8 %37 to i32
  %39 = icmp ne i32 %38, 0
  br i1 %39, label %40, label %50

40:                                               ; preds = %30
  %41 = load i32, i32* %1, align 4
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @neighbour_count, i64 0, i64 %42
  %44 = load i32, i32* %2, align 4
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds [100 x i8], [100 x i8]* %43, i64 0, i64 %45
  %47 = load i8, i8* %46, align 1
  %48 = zext i8 %47 to i32
  %49 = icmp eq i32 %48, 3
  br i1 %49, label %71, label %50

50:                                               ; preds = %40, %30
  %51 = load i32, i32* %1, align 4
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %52
  %54 = load i32, i32* %2, align 4
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds [100 x i8], [100 x i8]* %53, i64 0, i64 %55
  %57 = load i8, i8* %56, align 1
  %58 = icmp ne i8 %57, 0
  br i1 %58, label %69, label %59

59:                                               ; preds = %50
  %60 = load i32, i32* %1, align 4
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @neighbour_count, i64 0, i64 %61
  %63 = load i32, i32* %2, align 4
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds [100 x i8], [100 x i8]* %62, i64 0, i64 %64
  %66 = load i8, i8* %65, align 1
  %67 = zext i8 %66 to i32
  %68 = icmp eq i32 %67, 3
  br label %69

69:                                               ; preds = %59, %50
  %70 = phi i1 [ false, %50 ], [ %68, %59 ]
  br label %71

71:                                               ; preds = %69, %40, %20
  %72 = phi i1 [ true, %40 ], [ true, %20 ], [ %70, %69 ]
  %73 = zext i1 %72 to i32
  %74 = trunc i32 %73 to i8
  %75 = load i32, i32* %1, align 4
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds [200 x [100 x i8]], [200 x [100 x i8]]* @map, i64 0, i64 %76
  %78 = load i32, i32* %2, align 4
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds [100 x i8], [100 x i8]* %77, i64 0, i64 %79
  store i8 %74, i8* %80, align 1
  br label %81

81:                                               ; preds = %71
  %82 = load i32, i32* %2, align 4
  %83 = add nsw i32 %82, 1
  store i32 %83, i32* %2, align 4
  br label %7, !llvm.loop !12

84:                                               ; preds = %7
  br label %85

85:                                               ; preds = %84
  %86 = load i32, i32* %1, align 4
  %87 = add nsw i32 %86, 1
  store i32 %87, i32* %1, align 4
  br label %3, !llvm.loop !13

88:                                               ; preds = %3
  call void @update_neighbour_count()
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @init_map()
  br label %2

2:                                                ; preds = %0, %2
  call void @sim_clear(i32 -2139062017)
  call void @draw_map()
  call void (...) @sim_display()
  call void @update_map()
  br label %2
}

declare void @sim_clear(i32) #1

declare void @sim_display(...) #1

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

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
