# Mercy Programming Language

* Language specification [mercy.md](mercy.md)
* Compiler source code [src](src)
* Code example [Conway's life game](examples/life_game)


### Build and run tests
```bash
mkdir build && cd build
cmake ..
make
make test
```

### Usage
Compilation to llvm IR
```bash
echo 'let main() { print("Hello, world!"); };' | ./mercy --emit-llvm
echo 'let main() { print("Hello, world!"); };' | ./mercy --emit-llvm | lli
```
Compilation to object file
```bash
echo 'let main() { print("Hello, world!"); };' | ./mercy -c -o hello.o
clang hello.o -o hello
./hello
```
