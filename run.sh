#!/bin/bash

# Compile to .ll LLVM-IR
./parser $1 2>&1 | sed -u 1,/BEGIN_IR/d > $2

# Assemble IR
llc $2 -o out.s

# Compile and link with allocator
clang++ -c allocator.cpp
clang++ out.s allocator.o

# run and hope
./a.out




