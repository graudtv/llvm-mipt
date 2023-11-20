# generate 10 random digits

entry:
  ori s0, zero, 0xff01
  lui s0, s0, 0xffff # s0 = IO_STDOUT

  ori s1, zero, 0xff10
  lui s1, s1, 0xffff # s1 = SIM_BASE_ADDR

  ori s2, zero, 0x20 # s2 = ' '

  xor t0, t0, t0 # t0 = i = 0
  addi t1, zero, 10 # t1 = count = 10
loop: # while (i < count)
  bgt t0, t1, exit
  load t2, s1, 5 # t2 = r = sim_rand()
  remiu t2, t2, 10 # r = r % 10
  addi t2, t2, 48 # r = r + '0'
  store t2, s0, 0 # putchar(r)
  store s2, s0, 0 # puchar(' ')
  addi t0, t0, 1 # ++i
  jalr zero, rpc, loop
exit:
  ori t2, zero, 0x0a # '\n'
  store t2, s0, 0
