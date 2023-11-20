jalr zero, rpc, entry

# --- static data begin ---
COLOR_SCHEME:
  .word -253811457
  .word 1659905279
  .word 619763455
  .word 616755455
  .word 607318271
  .word -1024659201
  .word -266068737
  .word -264493825
  .word -260430593

# Width and height must be the same as in sim.h
MAP_WIDTH: .word 200
MAP_HEIGHT: .word 100
BACKGROUND_COLOR: .word 0x808080ff
INITIAL_DENSITY_RATIO: .word 5
SIM_BASE_ADDR: .word 0xffffff10
# --- static data end ---

entry:
  load s0, rpc, MAP_HEIGHT # s0 = MAP_HEIGHT
  load s1, rpc, MAP_WIDTH  # s1 = MAP_WIDTH
  mul s4, s0, s1 # s4 = MAP_SIZE = MAP_HEIGHT * MAP_WIDTH

  # allocate map on stack
  or s2, zero, rsp # s2 = map.is_alive
  muli t0, s4, 4
  add rsp, rsp, t0 # rsp += MAP_SIZE * 4
  or s3, zero, rsp # s3 = map.neighbour_count
  add rsp, rsp, t0 # rsp += MAP_SIZE * 4
  load s5, rpc, SIM_BASE_ADDR # s5 = SIM_BASE_ADDR

  jalr zero, rpc, init_map
main_loop:
  jalr zero, rpc, update_neighbour_count
main_loop_draw:
  # sim_clear()
  load t0, rpc, BACKGROUND_COLOR # t0 = BACKGROUND_COLOR
  store t0, s5, 0 # sim_clear(BACKGROUND_COLOR)
  # draw_map()
  jalr zero, rpc, draw_map
main_loop_display:
  store zero, s5, 1 # sim_display()
  ### To enable static mode, comment the next line
  jalr zero, rpc, update_map # update_map()
  jalr zero, rpc, main_loop

# void draw_map()
draw_map:
  xor t0, t0, t0 # t0 = i = 0
  or a0, zero, s2 # a0 = ptr_alive = map.is_alive[0][0]
  or a1, zero, s3 # a1 = ptr_neighbour = map.neighbour_count[0][0]
  addi a2, rpc, COLOR_SCHEME # a2 = &color_scheme[0]
  # WA: I have forgotten to add left shift instr to arch, so use mul for
  # encoding pixel coordinates :)
  xor a3, a3, a3
  lui a3, a3, 1 # t7 = 1 << 16
.draw_map_cond_i:
  bge t0, s0, .draw_map_exit_i # while (i < MAP_HEIGHT)
.draw_map_loop_i:
  xor t1, t1, t1 # t1 = j = 0
.draw_map_cond_j:
  bge t1, s1, .draw_map_exit_j # while (j < MAP_WIDTH)
.draw_map_loop_j:
  load t3, a0, 0 # t3 = map.is_alive[i][j]
  beq t3, zero, .draw_map_inc_j # if (map.is_alive[i][j])
  load t4, a1, 0 # t4 = map.neighbour_count[i][j]
  muli t4, t4, 4 # t4 = offset_in_color_scheme
  add t4, a2, t4 # t4 = &color_scheme[map.neighbour_count[i][j]]
  load t4, t4, 0 # t4 = color_scheme[map.neighbour_count[i][j]]
  store t4, s5, 2 # *SIM_PIXEL_COLOR = color_scheme[map.neighbour_count[i][j]]

  mul t4, t1, a3 # t5 = j << 16
  or t4, t4, t0 # t5 = (j << 16) | i
  store t4, s5, 4 # sim_set_pixel(j, i)

.draw_map_inc_j:
  addi a0, a0, 4 # ptr_alive += 4
  addi a1, a1, 4 # ptr_neighbour += 4
  addi t1, t1, 1 # ++j
  jalr zero, rpc, .draw_map_cond_j
.draw_map_exit_j:
  addi t0, t0, 1 # ++i
  jalr zero, rpc,  .draw_map_cond_i
.draw_map_exit_i:
  jalr zero, rpc, main_loop_display

# void update_neighbour_count()
update_neighbour_count:
  xor t0, t0, t0 # t0 = i = 0
  or t2, zero, s3 # ptr_neighbour = &map.neighbour_count[0][0]
.update_neighbour_count_cond_i:
  bge t0, s0, .update_neighbour_count_exit_i # while (i < MAP_HEIGHT)
.update_neighbour_count_loop_i:
  xor t1, t1, t1 # t1 = j = 0
.update_neighbour_count_cond_j:
  bge t1, s1, .update_neighbour_count_exit_j # while (j < MAP_WIDTH)
.update_neighbour_count_loop_j:
  # calculate left index
  add t3, t1, s1 # j + MAP_WIDTH
  subi t3, t3, 1 # j + MAP_WIDTH - 1
  remu t3, t3, s1 # t3 = left = (j + MAP_WIDTH - 1) % MAP_WIDTH
  muli t3, t3, 4 # t3 = left_offset = left * 4
  # calculat right index
  addi t4, t1, 1 # j + 1
  remu t4, t4, s1 # t4 = right = (j + 1) % MAP_WIDTH
  muli t4, t4, 4 # t4 = right_offset = right * 4
  # calculate top index
  add t5, t0, s0 # i + MAP_HEIGHT
  subi t5, t5, 1 # j + MAP_HEIGHT - 1
  remu t5, t5, s0 # t5 = top = (j + MAP_HEIGHT - 1) % MAP_HEIGHT
  mul t5, t5, s1
  muli t5, t5, 4 # t5 = top_offset = top * MAP_WIDTH * 4
  # calculate bottom index
  addi t6, t0, 1 # i + 1
  remu t6, t6, s0 # t6 = bottom = (i + 1) % MAP_HEIGHT
  mul t6, t6, s1
  muli t6, t6, 4 # t6 = bottom_offset = bottom * MAP_WIDTH * 4

  # accumulate count of alive neighbours in a0
  # upper row
  add t7, s2, t5 # t7 = &map.is_alive[top][0]
  add a1, t7, t3 # a1 = &map.is_alive[top][left]
  load a0, a1, 0 # a0 = map.is_alive[top][left]

  muli a1, t1, 4 # a1 = j * 4
  add a1, t7, a1 # a1 = &map.is_alive[top][j]
  load a1, a1, 0 # a1 = &map.is_alive[top][j]
  add a0, a0, a1 # a0 += map.is_alive[top][j]

  add a1, t7, t4 # a1 = &map.is_alive[top][right]
  load a1, a1, 0 # a1 = &map.is_alive[top][right]
  add a0, a0, a1 # a0 += map.is_alive[top][right]

  # middle row
  mul t7, t0, s1 # t7 = i * MAP_WIDTH
  muli t7, t7, 4 # t7 = i * MAP_WIDTH * 4
  add t7, s2, t7 # t7 = &map.is_alive[i][0]
  add a1, t7, t3 # a1 = &map.is_alive[i][left]
  load a1, a1, 0 # a1 = &map.is_alive[i][left]
  add a0, a0, a1 # a0 += map.is_alive[i][j]

  add a1, t7, t4 # a1 = &map.is_alive[i][right]
  load a1, a1, 0 # a1 = &map.is_alive[i][right]
  add a0, a0, a1 # a0 += map.is_alive[i][right]
  
  # bottom row
  add t7, s2, t6 # t7 = &map.is_alive[bottom][0]
  add a1, t7, t3 # a1 = &map.is_alive[bottom][left]
  load a1, a1, 0 # a1 = &map.is_alive[bottom][left]
  add a0, a0, a1 # a0 += map.is_alive[bottom][j]

  muli a1, t1, 4 # a1 = j * 4
  add a1, t7, a1 # a1 = &map.is_alive[bottom][j]
  load a1, a1, 0 # a1 = &map.is_alive[bottom][j]
  add a0, a0, a1 # a0 += map.is_alive[bottom][j]

  add a1, t7, t4 # a1 = &map.is_alive[bottom][right]
  load a1, a1, 0 # a1 = &map.is_alive[bottom][right]
  add a0, a0, a1 # a0 += map.is_alive[bottom][right]

  # store result to map.neighbour_count[i][j]
  store a0, t2, 0
  or rv0, zero, a0

  addi t2, t2, 4 # ptr_neighbour += 4
  addi t1, t1, 1 # ++j
  jalr zero, rpc, .update_neighbour_count_cond_j
.update_neighbour_count_exit_j:
  addi t0, t0, 1 # ++i
  jalr zero, rpc,  .update_neighbour_count_cond_i
.update_neighbour_count_exit_i:
  jalr zero, rpc, main_loop_draw

# void update_map()
update_map:
  xor t0, t0, t0 # t0 = linear_idx = 0
  or a0, zero, s2 # a0 = ptr_alive = &map.is_alive[0][0]
  or a1, zero, s3 # a1 = ptr_neighbour = &map.neighbour_count[0][0]
.update_map_cond:
  bge t0, s4, .update_map_exit # while (linear_idx < MAP_SIZE)
.update_map_loop:
  load t6, a0, 0 # t6 = map.is_alive[i][j]
  load t7, a1, 0 # t7 = map.neighbour_count[i][j]
  beq t6, zero, .update_map_cell_was_dead # if (map.is_alive[i][j])
  ori t2, zero, 2
  beq t7, t2, .update_map_set_alive
  ori t2, zero, 3
  beq t7, t2, .update_map_set_alive
  jalr zero, rpc, .update_map_set_dead
.update_map_cell_was_dead:
  ori t2, zero, 3
  beq t7, t2, .update_map_set_alive
.update_map_set_dead:
  xor t2, t2, t2
  store t2, a0, 0 # map.is_alive[i][j] = 0
  jalr zero, rpc, .update_map_inc
.update_map_set_alive:
  addi t2, zero, 1
  store t2, a0, 0 # map.is_alive[i][j] = 1
.update_map_inc:
  # increment index
  addi a0, a0, 4 # ptr_alive += 4
  addi a1, a1, 4 # ptr_neighbour += 4
  addi t0, t0, 1 # ++linear_idx
  jalr zero, rpc, .update_map_cond
.update_map_exit:
  jalr zero, rpc, main_loop

# void init_map()
init_map:
  xor t0, t0, t0 # t0 = linear_idx = 0
  or t7, zero, s2 # t7 = ptr = &map.is_alive[0]
  load t4, rpc, INITIAL_DENSITY_RATIO # t4 = INITIAL_DENSITY_RATIO
.init_map_cond:
  bge t0, s4, .init_map_exit # while (linear_idx < MAP_SIZE)
.init_map_loop:
  # get random value
  load t6, s5, 5 # t6 = rand()
  remu t6, t6, t4 # t6 = rand() % INITIAL_DENSITY_RATIO
  sltiu t6, t6, 1 # t6 = (rand() % INITIAL_DENSITY_RATIO) == 0
  store t6, t7, 0
  # increment index
  addi t7, t7, 4 # ptr += 4
  addi t0, t0, 1 # ++linear_idx
  jalr zero, rpc, .init_map_cond
.init_map_exit:
  jalr zero, rpc, main_loop
