# Prints "Hello, world!"

ori t0, zero, 0xff01
lui t0, t0, 0xffff # t0 = IO_STDOUT

ori t1, zero, 0x48 # 'H'
store t1, t0, 0

ori t1, zero, 0x65 # 'e'
store t1, t0, 0

ori t1, zero, 0x6c # 'l'
store t1, t0, 0
store t1, t0, 0

ori t1, zero, 0x6f # 'o'
store t1, t0, 0

ori t1, zero, 0x2c # ','
store t1, t0, 0

ori t1, zero, 0x20 # ' '
store t1, t0, 0

ori t1, zero, 0x77 # 'w'
store t1, t0, 0

ori t1, zero, 0x6f # 'o'
store t1, t0, 0

ori t1, zero, 0x72 # 'r'
store t1, t0, 0

ori t1, zero, 0x6c # 'l'
store t1, t0, 0

ori t1, zero, 0x64 # 'd'
store t1, t0, 0

ori t1, zero, 0x21 # '!'
store t1, t0, 0

ori t1, zero, 0x0a # '\n'
store t1, t0, 0
