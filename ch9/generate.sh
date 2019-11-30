#!/bin/bash

# Ka language to LLVM IR
./toy $1".ka" 2> $1".ll"


# LLVM IR(text) compile to nativa object file
clang -x ir $1."ll" -c -o $1".o"

#link and generate execute file
clang $1".o" printd.o -o $1

#execute
./$1
