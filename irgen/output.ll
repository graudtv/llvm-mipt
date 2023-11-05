; ModuleID = 'top'
source_filename = "top"

%struct.map_t = type { [100 x [200 x i8]], [100 x [200 x i8]] }

@color_scheme = private global [9 x i32] [i32 -253811457, i32 1659905279, i32 619763455, i32 616755455, i32 607318271, i32 -1024659201, i32 -266068737, i32 -264493825, i32 -260430593]

declare void @sim_clear(i32)

declare void @sim_display()

declare void @sim_set_pixel(i32, i32, i32, i32)

declare i32 @sim_rand()

define void @update_neighbour_count(%struct.map_t* %map) {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %cond_i

cond_i:                                           ; preds = %exit_j, %entry
  %ival = load i32, i32* %i, align 4
  %icmp = icmp slt i32 %ival, 100
  br i1 %icmp, label %loop_i, label %exit_i

cond_j:                                           ; preds = %loop_j, %loop_i
  %jval = load i32, i32* %j, align 4
  %jcmp = icmp slt i32 %jval, 200
  br i1 %jcmp, label %loop_j, label %exit_j

loop_i:                                           ; preds = %cond_i
  store i32 0, i32* %j, align 4
  br label %cond_j

loop_j:                                           ; preds = %cond_j
  %left_idx.tmp = add i32 %jval, 199
  %left_idx = urem i32 %left_idx.tmp, 200
  %right_idx.tmp = add i32 %jval, 1
  %right_idx = urem i32 %right_idx.tmp, 200
  %top_idx.tmp = add i32 %ival, 99
  %top_idx = urem i32 %top_idx.tmp, 100
  %bottom_idx.tmp = add i32 %ival, 1
  %bottom_idx = urem i32 %bottom_idx.tmp, 100
  %top_left_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %top_idx, i32 %left_idx
  %top_left = load i8, i8* %top_left_ptr, align 1
  %top_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %top_idx, i32 %jval
  %top = load i8, i8* %top_ptr, align 1
  %top_right_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %top_idx, i32 %right_idx
  %top_right = load i8, i8* %top_right_ptr, align 1
  %left_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %ival, i32 %left_idx
  %center_left = load i8, i8* %left_ptr, align 1
  %right_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %ival, i32 %right_idx
  %right = load i8, i8* %right_ptr, align 1
  %bottom_left_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %bottom_idx, i32 %left_idx
  %bottom_left = load i8, i8* %bottom_left_ptr, align 1
  %bottom_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %bottom_idx, i32 %jval
  %bottom = load i8, i8* %bottom_ptr, align 1
  %bottom_right_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %bottom_idx, i32 %right_idx
  %bottom_right = load i8, i8* %bottom_right_ptr, align 1
  %tmp.1 = add i8 %top_left, %top
  %tmp.2 = add i8 %top_right, %center_left
  %tmp.12 = add i8 %tmp.1, %tmp.2
  %tmp.3 = add i8 %right, %bottom_left
  %tmp.4 = add i8 %bottom, %bottom_right
  %tmp.34 = add i8 %tmp.3, %tmp.4
  %neighbour_count = add i8 %tmp.12, %tmp.34
  %neighbour_count_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 1, i32 %ival, i32 %jval
  store i8 %neighbour_count, i8* %neighbour_count_ptr, align 1
  %jval.inc = add i32 %jval, 1
  store i32 %jval.inc, i32* %j, align 4
  br label %cond_j

exit_i:                                           ; preds = %cond_i
  ret void

exit_j:                                           ; preds = %cond_j
  %ival.inc = add i32 %ival, 1
  store i32 %ival.inc, i32* %i, align 4
  br label %cond_i
}

define void @init_map(%struct.map_t* %map) {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %cond_i

cond_i:                                           ; preds = %exit_j, %entry
  %ival = load i32, i32* %i, align 4
  %icmp = icmp slt i32 %ival, 100
  br i1 %icmp, label %loop_i, label %exit_i

cond_j:                                           ; preds = %loop_j, %loop_i
  %jval = load i32, i32* %j, align 4
  %jcmp = icmp slt i32 %jval, 200
  br i1 %jcmp, label %loop_j, label %exit_j

loop_i:                                           ; preds = %cond_i
  store i32 0, i32* %j, align 4
  br label %cond_j

loop_j:                                           ; preds = %cond_j
  %rand = call i32 @sim_rand()
  %srem = srem i32 %rand, 5
  %is_alive = icmp eq i32 %srem, 0
  %is_alive.i8 = zext i1 %is_alive to i8
  %elem = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %ival, i32 %jval
  store i8 %is_alive.i8, i8* %elem, align 1
  %jval.inc = add i32 %jval, 1
  store i32 %jval.inc, i32* %j, align 4
  br label %cond_j

exit_i:                                           ; preds = %cond_i
  call void @update_neighbour_count(%struct.map_t* %map)
  ret void

exit_j:                                           ; preds = %cond_j
  %ival.inc = add i32 %ival, 1
  store i32 %ival.inc, i32* %i, align 4
  br label %cond_i
}

define void @draw_map(%struct.map_t* %map) {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %cond_i

cond_i:                                           ; preds = %exit_j, %entry
  %ival = load i32, i32* %i, align 4
  %icmp = icmp slt i32 %ival, 100
  br i1 %icmp, label %loop_i, label %exit_i

cond_j:                                           ; preds = %inc_j, %loop_i
  %jval = load i32, i32* %j, align 4
  %jcmp = icmp slt i32 %jval, 200
  br i1 %jcmp, label %loop_j, label %exit_j

loop_i:                                           ; preds = %cond_i
  store i32 0, i32* %j, align 4
  br label %cond_j

loop_j:                                           ; preds = %cond_j
  %is_alive_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %ival, i32 %jval
  %is_alive = load i8, i8* %is_alive_ptr, align 1
  %cmp = icmp ne i8 %is_alive, 0
  br i1 %cmp, label %draw, label %inc_j

exit_i:                                           ; preds = %cond_i
  call void @update_neighbour_count(%struct.map_t* %map)
  ret void

exit_j:                                           ; preds = %cond_j
  %ival.inc = add i32 %ival, 1
  store i32 %ival.inc, i32* %i, align 4
  br label %cond_i

inc_j:                                            ; preds = %draw, %loop_j
  %jval.inc = add i32 %jval, 1
  store i32 %jval.inc, i32* %j, align 4
  br label %cond_j

draw:                                             ; preds = %loop_j
  %neighbour_count_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 1, i32 %ival, i32 %jval
  %neighbour_count = load i8, i8* %neighbour_count_ptr, align 1
  %color_ptr = getelementptr inbounds [9 x i32], [9 x i32]* @color_scheme, i32 0, i8 %neighbour_count
  %color = load i32, i32* %color_ptr, align 4
  call void @sim_set_pixel(i32 %jval, i32 %ival, i32 %color, i32 32)
  br label %inc_j
}

define void @update_map(%struct.map_t* %map) {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %cond_i

cond_i:                                           ; preds = %exit_j, %entry
  %ival = load i32, i32* %i, align 4
  %icmp = icmp slt i32 %ival, 100
  br i1 %icmp, label %loop_i, label %exit_i

cond_j:                                           ; preds = %loop_j, %loop_i
  %jval = load i32, i32* %j, align 4
  %jcmp = icmp slt i32 %jval, 200
  br i1 %jcmp, label %loop_j, label %exit_j

loop_i:                                           ; preds = %cond_i
  store i32 0, i32* %j, align 4
  br label %cond_j

loop_j:                                           ; preds = %cond_j
  %is_alive_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 0, i32 %ival, i32 %jval
  %is_alive = load i8, i8* %is_alive_ptr, align 1
  %alive = icmp ne i8 %is_alive, 0
  %not_alive = xor i1 %alive, true
  %neighbour_count_ptr = getelementptr inbounds %struct.map_t, %struct.map_t* %map, i32 0, i32 1, i32 %ival, i32 %jval
  %neighbour_count = load i8, i8* %neighbour_count_ptr, align 1
  %ncount_eq_2 = icmp eq i8 %neighbour_count, 2
  %ncount_eq_3 = icmp eq i8 %neighbour_count, 3
  %cond1 = and i1 %alive, %ncount_eq_2
  %cond2 = and i1 %alive, %ncount_eq_3
  %cond3 = and i1 %not_alive, %ncount_eq_3
  %tmp = or i1 %cond1, %cond2
  %cond = or i1 %tmp, %cond3
  %set_alive = zext i1 %cond to i8
  store i8 %set_alive, i8* %is_alive_ptr, align 1
  %jval.inc = add i32 %jval, 1
  store i32 %jval.inc, i32* %j, align 4
  br label %cond_j

exit_i:                                           ; preds = %cond_i
  call void @update_neighbour_count(%struct.map_t* %map)
  ret void

exit_j:                                           ; preds = %cond_j
  %ival.inc = add i32 %ival, 1
  store i32 %ival.inc, i32* %i, align 4
  br label %cond_i
}

define i32 @main() {
entry:
  %map = alloca %struct.map_t, align 8
  call void @init_map(%struct.map_t* %map)
  br label %loop

loop:                                             ; preds = %loop, %entry
  call void @sim_clear(i32 -2139062017)
  call void @draw_map(%struct.map_t* %map)
  call void @sim_display()
  call void @update_map(%struct.map_t* %map)
  br label %loop
}
