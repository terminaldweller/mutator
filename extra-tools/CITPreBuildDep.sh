#!/bin/bash

"sudo" apt-get -qq update

#installing libstdc++ 4.8
"gcc" -v
"sudo" add-apt-repository ppa:ubuntu-toolchain-r/test
"sudo" apt-get update
"sudo" update-alternatives --remove-all gcc
"sudo" update-alternatives --remove-all g++
"sudo" apt-get install gcc-4.8
"sudo" apt-get install g++-4.8
"sudo" update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 20
"sudo" update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 20
"sudo" update-alternatives --config gcc
"sudo" update-alternatives --config g++
"sudo" apt-get update
"sudo" apt-get upgrade -y
"sudo" apt-get dist-upgrade
"gcc" -v

#building cmkae 3.4.3 since the CI Travis virtual host is Ubuntu(???)
"wget" http://www.cmake.org/files/v3.4/cmake-3.4.3.tar.gz
"tar" -xvzf cmake-3.4.3.tar.gz 
"cd" cmake-3.4.3/
"./configure"
"make"
"sudo" make install -j8
"sudo" update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 1 --force
"cd" ..

#building llvm and clang 4.0.0
"mkdir" llvm-clang4
"cd" llvm-clang4
"svn" co http://llvm.org/svn/llvm-project/llvm/trunk llvm
"mkdir" build
"cd" ./llvm/tools
"svn" co http://llvm.org/svn/llvm-project/cfe/trunk clang
"cd" ../..
"cd" build
"cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../llvm
"make" -j8
"sudo" make install
"cd" ../..