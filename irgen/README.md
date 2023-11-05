### Build
```bash
mkdir build && cd build
cmake ..
make
```

### Run
```bash
# print IR
./irgen
# print IR and run it with ExecutionEngine
./irgen --run
```

### Notes
As a reference, output.ll contains IR generated with llvm-14
