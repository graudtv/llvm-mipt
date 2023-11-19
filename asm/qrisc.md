# qrisc specification

### About

qrisc (cute risc) is a minimalistic 32-bit risc architecture for research
purposes

Features:
* 32-bit arithmetics and addressing
* 32 registers
* 30 instructions

### List of instructions

```
| mnemonic | encoding |
|----------|----------|
| add      | 0x01     |
| addi     | 0x02     |
| and      | 0x03     |
| andi     | 0x04     |
| or       | 0x05     |
| ori      | 0x06     |
| xor      | 0x07     |
| xori     | 0x08     |
| sub      | 0x09     |
| subi     | 0x0a     |
| mul      | 0x0b     |
| muli     | 0x0c     |
| divu     | 0x0d     |
| diviu    | 0x0e     |
| remu     | 0x0f     |
| remiu    | 0x10     |
| slt      | 0x11     |
| slti     | 0x12     |
| sltu     | 0x13     |
| sltiu    | 0x14     |
| lui      | 0x15     |
|          |          |
| load     | 0x16     |
| store    | 0x17     |
|          |          |
| beq      | 0x28     |
| bne      | 0x2a     |
| bgt      | 0x2a     |
| bge      | 0x2b     |
| blt      | 0x2c     |
| ble      | 0x2d     |
| jalr     | 0x2e     |
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

##### Arithemetic Instructions
```
opcodes: add, or, and, xor, sub, mul, divu, remu, slt, sltu
type: R
semantics: r1 = r2 <op> r3
description: binary operation on 2 registers
```

```
opcodes: addi, ori, andi, xori, subi, muli, diviu, remiu, slti
type: I
semantics: r1 = r2 <op> sign_extend(imm)
description: binary operation on register and immediate
```

```
opcode: remiu, sltiu
type: I
semantics: r1 = r2 <op> zero_extend(imm)
description: binary operation on register and immediate
```

```
opcode: lui
type: I
semantics: r1 = r2 | (zero_extend(imm) & 0x00ff << 16)
description: set 16 upper bits to imm
```

##### Memory Instructions

```
opcode: load
type: I
semantics: r1 = mem[r2 + zero_extend(imm)]
description: load value from memory to register
```

```
opcode: store
type: I
semantics: mem[r2 + zero_extend(imm)] = r1
description: save value from register to memory
```

##### Control Flow Instructions

```
opcodes: beq, bne, bgt, bge, blt, ble
type: I
semantics: next_pc = (r1 <op> r2) ? (pc + sign_extend(imm) * 4) : pc
description: jump by signed offset to pc if condition is satisfied
```

```
opcode: jalr
type: I
semantics: (r1, next_pc) = (pc + 4, r2 + sign_extend(imm) * 4)
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
| mv rdst, rsrc   | xor rdst, rsrc, zero   |

Macro commands: .word value
```

