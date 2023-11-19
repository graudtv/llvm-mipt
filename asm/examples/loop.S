# print numbers from 0 to 9

entry:
  ori s0, zero, 0xff01
  lui s0, s0, 0xffff # s0 = IO_STDOUT

  ori s1, zero, 0x20 # s2 = ' '

  xor t0, t0, t0 # t0 = i = 0
  addi t1, zero, 9

loop:
  bgt t0, t1, 6 # while (i < 9)
  addi t2, t0, 48 # char c = i + '0'
  store t2, s0, 0 # putchar(c)
  store s1, s0, 0 # puchar(' ')
  addi t0, t0, 1 # ++i
  jalr zero, rpc, -5
  # jalr here is equivalent to 'beq zero, zero, -5'

exit:
  ori t2, zero, 0x0a # '\n'
  store t2, s0, 0
