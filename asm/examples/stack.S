# example of stack usage

init_stack:
  ori s0, zero, 0xff01
  lui s0, s0, 0xffff # t0 = IO_STDOUT

  ori t0, zero, 0x48 # 'H'
  ori t1, zero, 0x65 # 'e'
  ori t2, zero, 0x6c # 'l'
  ori t3, zero, 0x6f # 'o'
  ori t4, zero, 0x21 # '!'

  store t0, rsp, 0 # 'H'
  store t1, rsp, 4 # 'e'
  store t2, rsp, 8 # 'l'
  store t2, rsp, 12 # 'l'
  store t3, rsp, 16 # 'o'
  store t4, rsp, 20 # '!'
  store zero, rsp, 24 # '\0'

  xor t0, t0, t0
loop: # while (c != '\0')
  add t1, rsp, t0 # char *ptr = stack[offset]
  load t1, t1, 0 # t1 = c = *ptr
  beq t1, zero, exit
  store t1, s0, 0 # putchar(c)
  addi t0, t0, 4 # ptr += 4
  jalr zero, rpc, loop
exit:
  ori t1, zero, 0x0a # '\n'
  store t1, s0, 0
