#!/bin/bash
cd $(dirname $0)
"./obfuscator" --shake 256 --shake_len 44 ./test/test.cpp
"./obfuscator" --shake 256 --shake_len 44 ./test/header.hpp --
"g++" ./FILE15118982290295364091.cpp
#expected to return 128
./a.out
