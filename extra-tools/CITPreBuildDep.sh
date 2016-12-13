#!/bin/bash

"sudo" apt-get -qq update

"sudo" apt-get install libstdc++

#building cmkae 3.4.3 since the CI Travis virtual host is Ubuntu(???)
"wget" http://www.cmake.org/files/v3.4/cmake-3.4.3.tar.gz
"tar" -xvzf cmake-3.4.3.tar.gz 
"cd" cmake-3.4.3/
"./configure"
"make"
"sudo" make install
"sudo" update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 1 --force

#building llvm and clang 4.0.0
"cd" ~
"mkdir" llvm-clang4
"cd" llvm-clang4
"svn" co http://llvm.org/svn/llvm-project/llvm/trunk llvm
"mkdir" build
"cd" ./llvm/tools
"svn" co http://llvm.org/svn/llvm-project/cfe/trunk clang
"cd" ../..
"cd" build
"cmake" -G "Unix Makefiles" ../llvm -DC_BUILD_TYPE=Release
"make"
