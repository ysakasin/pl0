#!/bin/sh
./build/llvmpl0 $1
llvm-link out.ll ./build/write.ll -S -o ./build/linked.ll
opt -S -mem2reg ./build/linked.ll > out.ll
