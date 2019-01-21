#!/bin/bash

# Compile to .ll LLVM-IR
./parser $1 2>&1 | sed -u 1,/BEGIN_IR/d > build/out.ll

# Perform statepoint relocation opt pass
opt -rewrite-statepoints-for-gc -S -o build/out-sp.ll build/out.ll

# # Assemble IR
llc -o build/out-sp.s build/out-sp.ll


# # Compile and link with allocator
clang -c -o build/statepoint.o runtime/includes/statepoint.c
clang++ -c -o build/allocator.o runtime/allocator.cpp
clang++ -c -o build/out-sp.o build/out-sp.s  
objcopy --globalize-symbol=__LLVM_StackMaps build/out-sp.o build/out-sp-globalised.o
clang++ -o build/a.out build/out-sp-globalised.o build/allocator.o build/statepoint.o

# # run and hope
# ./build/a.out




