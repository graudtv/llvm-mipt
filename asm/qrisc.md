# qrisc specification

### About

qrisc (cute risc) is a minimalistic 32-bit risc architecture for research
purposes

### List of instructions

```
| opcode  | encoding |
|---------|----------|
| add     | 0x04     |
| addi    | 0x05     |
| addm    | 0x06     |
|         |          |
| and     | 0x08     |
| andi    | 0x09     |
| andm    | 0x0a     |
|         |          |
| or      | 0x0c     |
| ori     | 0x0d     |
| orm     | 0x0e     |
|         |          |
| xor     | 0x10     |
| xori    | 0x11     |
| xorm    | 0x12     |
|         |          |
| urem    | 0x14     |
| uremi   | 0x15     |
| uremm   | 0x16     |
|         |          |
| load    | 0x18     |
| store   | 0x19     |
|         |          |
| lli     | 0x1a     |
| lui     | 0x1b     |
|         |          |
| bz      | 0xf0     |
| bnz     | 0xf1     |
| be      | 0xf2     |
| bne     | 0xf3     |
| bgt     | 0xf4     |
| bge     | 0xf5     |
| blt     | 0xf6     |
| ble     | 0xf7     |
|         |          |
| jalr    | 0xf8     |
```

Pseudo instructions:
```
| pseudo instr    |       equivalent       |
|-----------------|------------------------|
| jr reg, offset  | jalr zero, reg, offset |
| ret             | jalr zero, ra, 0       |
| push reg        | store reg, rsp, 0      |
| pop reg         | load reg, rsp, 0       |
| mv rdst, rsrc   | or rdst, rsrc, zero    |
```

### Instruction Encoding
```
R(egister) type

 31            24 23            16 15             8 7              0
---------------------------------------------------------------------
|     opcode     |      r1        |       r2       |       r3       |
---------------------------------------------------------------------
```

```
I(mmediate) type

 31            24 23            16 15             8 7              0
---------------------------------------------------------------------
|     opcode     |      r1        |      r2        |      imm       |
---------------------------------------------------------------------
```

```
WI type (wide immediate)

 31            24 23            16 15                              0
---------------------------------------------------------------------
|     opcode     |      r1        |               imm               |
---------------------------------------------------------------------
```

### Semantics

##### Arithemetic and Memory Instructions
```
opcodes: add, or, and, xor, urem
type: R
semantics: r1 = r2 <op> r3
```

```
opcodes: addi, ori, andi, xori, uremi
type: I
semantics: r1 = r2 <op> imm
```

```
opcodes: addm, orm, andm, xorm
type: I
semantics: r1 = r1 <op> mem[r2 + imm]
```

```
opcode: lli
type: WI
semantics: r1 = imm
description: set 16 lower bits to imm, set 16 upper bits to zero
```

```
opcode: lui
type: WI
semantics: r1 = r1 | (imm << 16)
description: set 16 upper bits to imm
```

```
opcode: load
type: I
semantics: r1 = mem[r2 + imm]
```

```
opcode: store
type: I
semantics: mem[r2 + imm] = r1
```


##### Control Flow Instructions

```
opcodes: bz, bnz, be, bne, bgt, bge, blt, ble
type: I
semantics: next_pc = (r1 <op> r2) ? (pc + offset) : pc
```

```
opcode: jalr
type: I
semantics: (r1, next_pc) = (pc + 4, r2 + imm * 4)
```

### ABI

| Register | ABI name | Description                              | Saver  |
|----------|----------|------------------------------------------|--------|
| r0       | zero     | Hard-wired zero                          |   -    |
| r1       | ra       | Function return address                  | Caller |
| r2-r3    | rv1, rv2 | Function return values                   | Caller |
| r4-r8    | a0-a4    | Function arguments                       | Caller |
| r9-r14   | t0-t5    | Temporary values                         | Caller |
| r15      | rsp      | Stack pointer                            | Callee |

* If function has more arguments or return values than number of associated
  registers, it is unspecified how they are passed
* Stack grows upwards
* When program starts, rsp is required to point to at least 512B of stack
  space. Values of other registers are unspecified


### External devices

External devices can be accessed with any memory instruction: load, store,
addm, orm, ...

In write operations to registers in descriptions below, it is assumed that
"arg" is 32-bit value written to register.

In read operations from registers in descriptions below, it is assumed that
"rval" is 32-bit value loaded from registers.


##### IO

| Register          | MMIO address | Mode |
|-------------------|--------------|------|
| IO_STDIN          | 0xffffff00   |  RO  |
| IO_STDOUT         | 0xffffff04   |  WO  |

Description:
```
IO_STDION: rval = getchar()
IO_STDOUT: putchar(arg & 0xf)
```


##### Display

| Register          | MMIO address | Mode |
|-------------------|--------------|------|
| SIM_CLEAR         | 0xffffff10   |  WO  |
| SIM_DISPLAY       | 0xffffff14   |  WO  |
| SIM_PIXEL_SHAPE   | 0xffffff18   |  RW  |
| SIM_PIXEL_COLOR   | 0xffffff1a   |  RW  |
| SIM_SET_PIXEL     | 0xffffff20   |  WO  |
| SIM_RAND          | 0xffffff24   |  RO  |

Description:
```
SIM_CLEAR: sim_clear(color = arg)
SIM_DISPLAY: sim_display() // arg ignored
SIM_PIXEL_SHAPE: buffer register
SIM_PIXEL_COLOR: buffer register
SIM_SET_PIXEL:
  sim_set_pixel(x = arg & 0xf0, y = arg & 0x0f,
                color = *(uint32 *) SIM_PIXEL_COLOR,
                shape = *(uint32 *) SIM_PIXEL_SHAPE)
SIM_RAND: rval = sim_rand()
```

