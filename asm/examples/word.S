jalr zero, rpc, entry

str:
  .word 0x48 # 'H'
  .word 0x65 # 'e'
  .word 0x6c # 'l'
  .word 0x6c # 'l'
  .word 0x6f # 'o'
  .word 0x2c # ','
  .word 0x20 # ' '
  .word 0x77 # 'w'
  .word 0x6f # 'o'
  .word 0x72 # 'r'
  .word 0x6c # 'l'
  .word 0x64 # 'd'
  .word 0x21 # '!'
  .word 0x0a # '\n'
  .word 0x0  # '\0'

entry:
  ori s0, zero, 0xff01
  lui s0, s0, 0xffff # s0 = IO_STDOUT

  addi s1, rpc, str # s1 = &str[0]

  xor t0, t0, t0 # t0 = i = 0
loop: # while (c != '\0')
  add t1, s1, t0 # char *ptr = &str[i]
  load t1, t1, 0 # t1 = c = *ptr
  beq t1, zero, exit
  store t1, s0, 0 # putchar(c)
  addi t0, t0, 4 # ptr += 4
  jalr zero, rpc, loop
exit:
