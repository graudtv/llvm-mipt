# qrisc assembler and simulator

Contents:
* Toy _qrisc_ architecture specification: [qrisc.md](qrisc.md)
* _qrisc-asm_, _qrisc-sim_ and _qrisc-llvm-sim_ tools
* A couple of assembler examples: [examples](examples)

### Build
```bash
mkdir build && cd build
cmake ..
make
```

### qrisc-asm
```bash
./qrisc-asm ../examples/hello.S --print
./qrisc-asm ../examples/hello.S -o hello.bin
```

### qrisc-sim
Precise qrisc simulator, should work with any valid assembler

```bash
./qrisc-sim ../examples/hello.S
./qrisc-sim ../examples/rand.S --trace --trace-reg --trace-mem 2>&1 >/dev/null
```

### qrisc-llvm-sim
_qrisc-llvm-sim_ is not able to handle generic indirect jumps, but suffice for simple application and works much faster than ```qrisc-sim```

```bash
./qrisc-llvm-sim ../examples/hello.S
./qrisc-llvm-sim ../examples/life.S --print-ir
```
