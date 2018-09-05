# pl0
A interpreter of PL/0'

More detail of PL/0' is available on [コンパイラ 作りながら学ぶ](https://www.ohmsha.co.jp/book/9784274221163/).


## Dependencies

- C++14
  - Clang is recommended
- CMake
- LLVM ^6.0.1
  - When use LLVM backend


## Build

Use CMake.

```
mkdir build
cd build
cmake ..
make
```

### Targets

- `pl0`: Build evaluator on own virtual machine
- `llvmpl0` : Build compiler to LLVM IR


## Run

### VM version

```
build/pl0 sample.plz
```

### LLVM version

```
sh pl0.sh sample.plz
lli out.ll
```
