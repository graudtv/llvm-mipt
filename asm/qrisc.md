# qrisc specification

### About

qrisc (cute risc) is a minimalistic 32-bit risc architecture for research
purposes

Features:
* 32 general purpose registers
* 25 instructions

### List of instructions

```
| opcode  | encoding |
|---------|----------|
| add     | 0x01     |
| addi    | 0x02     |
| addm    | 0x03     |
|         |          |
| and     | 0x04     |
| andi    | 0x05     |
| andm    | 0x06     |
|         |          |
| or      | 0x07     |
| ori     | 0x08     |
| orm     | 0x09     |
|         |          |
| xor     | 0x0a     |
| xori    | 0x0b     |
| xorm    | 0x0c     |
|         |          |
| urem    | 0x0d     |
| uremi   | 0x0e     |
| uremm   | 0x0f     |
|         |          |
| load    | 0x10     |
| store   | 0x11     |
| lui     | 0x12     |
|         |          |
| beq     | 0x28     |
| bne     | 0x2a     |
| bgt     | 0x2a     |
| bge     | 0x2b     |
| blt     | 0x2c     |
| ble     | 0x2d     |
| jalr    | 0x2e     |
```

Opcodes 0x30-0x3f reserved for future extensions.

### Instruction Types
```
R(egister) type

 31        26 25      21 20      16 15      11 10                       0
--------------------------------------------------------------------------
|   opcode   |    r1    |    r2    |    r3    |             0            |
--------------------------------------------------------------------------
```

```
I(mmediate) type

 31        26 25      21 20      16 15                                  0
--------------------------------------------------------------------------
|   opcode   |    r1    |    r2    |               imm                   |
--------------------------------------------------------------------------
```

### Semantics

##### Arithemetic and Memory Instructions
```
opcodes: add, or, and, xor, urem
type: R
semantics: r1 = r2 <op> r3
description: binary operation on 4 registers
```

```
opcodes: addi, ori, andi, xori, uremi
type: I
semantics: r1 = r2 <op> imm
description: binary operation on register and immediate. imm is signed for
             addi, unsigned for other instructions
```

```
opcodes: addm, orm, andm, xorm
type: I
semantics: r1 = r1 <op> mem[r2 + imm]
description: binary operation on register and value in memory. imm is unsigned
```

```
opcode: lui
type: I
semantics: r1 = r2 | (imm & 0x00ff << 16)
description: set 16 upper bits to imm
```

```
opcode: load
type: I
semantics: r1 = mem[r2 + imm]
description: load value from memory to register
```

```
opcode: store
type: I
semantics: mem[r2 + imm] = r1
description: save value from register to memory
```

##### Control Flow Instructions

```
opcodes: beq, bne, bgt, bge, blt, ble
type: I
semantics: next_pc = (r1 <op> r2) ? (pc + imm * 4) : pc
description: jump by signed offset to pc if condition is satisfied
```

```
opcode: jalr
type: I
semantics: (r1, next_pc) = (pc + 4, r2 + imm * 4)
description: unconditional indirect jump by signed offset
```

### ABI

| Register | ABI name | Description                              | Saver  |
|----------|----------|------------------------------------------|--------|
| r0       | zero     | Hard-wired zero                          |   -    |
| r1       | ra       | Function return address                  | Caller |
| r2-r5    | rv0-rv3  | Function return values                   | Caller |
| r6-r11   | a0-a5    | Function arguments                       | Caller |
| r12-r19  | t0-t7    | Temporary values                         | Caller |
| r20-r25  | s0-s5    | Save register                            | Callee |
| r26-r28  | Reserved | Reserved                                 |   -    |
| r29      | rsp      | Stack pointer                            | Callee |
| r30      | rbp      | Stack base pointer                       | Callee |
| r31      | rpc      | Hard-wired PC (read only)                |   -    |

* If function has more arguments or return values than number of associated
  registers, it is unspecified how they are passed
* Stack grows upwards
* When program starts, rsp is required to point to at least 512B of stack
  space. Values of other registers are unspecified


### External devices

External devices can be accessed via MMIO with any memory instruction: load,
store, addm, orm, etc. Addresses 0xffffff00-0xffffffff are reserved for MMIO.

In write operations to registers in descriptions below, it is assumed that
"arg" is 32-bit value written to register.

In read operations from registers in descriptions below, it is assumed that
"rval" is 32-bit value loaded from register.


##### IO

| Register          | MMIO address | Mode |
|-------------------|--------------|------|
| IO_STDIN          | 0xffffff00   |  RO  |
| IO_STDOUT         | 0xffffff01   |  WO  |

Description:
```
IO_STDION: rval = getchar()
IO_STDOUT: putchar(arg & 0xf)
```

##### Display

| Register          | MMIO address | Mode |
|-------------------|--------------|------|
| SIM_CLEAR         | 0xffffff10   |  WO  |
| SIM_DISPLAY       | 0xffffff11   |  WO  |
| SIM_PIXEL_COLOR   | 0xffffff12   |  RW  |
| SIM_PIXEL_SHAPE   | 0xffffff13   |  RW  |
| SIM_SET_PIXEL     | 0xffffff14   |  WO  |
| SIM_RAND          | 0xffffff15   |  RO  |

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

### Assembly language

##### Notation
```
<label>:                          # label
  <mnemonic> r1, r2, r3           # R-type instruction
  <mnemonic> r1, r2, imm          # I-type instruction
```

Label names can be used as immediates. Meaning depends on the instruction:

- When used in jump and branch instructions, imm is set to offset in bytes of
  the label from the current instruction devided by 4, so that the jump is
  performed on the instruction under the label
- When used in arithmetic instructions, imm is set to offset in bytes of the
  label from the current instruction

Examples:
```
  j label
  ...
label:
```

```
  val1: .i32 0x12345678
  val2: .i32 0xffffffff
  ...
  addi t0, rpc, value      # t0 = &val1
  load t1, t0, 0           # t1 = val1 = 0x12345678
  load t2, t0, 4           # t2 = val2 = 0xffffffff
```

##### Pseudo intructions
```
| pseudo instr    |       equivalent       |
|-----------------|------------------------|
| j offset        | jal zero, offset       |
| jr reg, offset  | jalr zero, reg, offset |
| bz reg, offset  | beq reg, zero, offset  |
| bnz reg, offset | bne reg, zero, offset  |
| ret             | jalr zero, ra, 0       |
| push reg        | store reg, rsp, 0      |
| pop reg         | load reg, rsp, 0       |
| mv rdst, rsrc   | or rdst, rsrc, zero    |
```

