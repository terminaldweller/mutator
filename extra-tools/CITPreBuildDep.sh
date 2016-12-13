#!/bin/bash

"sudo apt-get" install cmake -y
"cd" ~
"mkdir" llvm-clang4
"cd" llvm-clang4
"svn" co http://llvm.org/svn/llvm-project/llvm/trunk llvm
"mkdir" build
"cd" ./llvm/tools
"svn" co http://llvm.org/svn/llvm-project/cfe/trunk clang
"cd" ../..
"cd" build
"cmake" -G "Unix Makefiles" ../llvm -DC_MAKE_TYPE=Release
"make"
