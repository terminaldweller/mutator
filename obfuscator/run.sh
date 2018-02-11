#!/bin/bash
cd $(dirname $0)
"./obfuscator" ./test/test.cpp
"./obfuscator" ./test/header.hpp --
"g++" ./FILE15118982290295364091.cpp
#expected to return 128
./a.out
