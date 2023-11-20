# show red screen

entry:
  ori s0, zero, 0xff10
  lui s0, s0, 0xffff # s1 = SIM_BASE_ADDR

  ori s1, s1, 0x00ff
  lui s1, s1, 0xff00 # s1 = rgb_green = 0xff0000ff

loop:
  store s1, s0, 0 # sim_clear(rgb_green)
  store zero, s0, 1 # sim_display()
  jalr zero, rpc, loop
