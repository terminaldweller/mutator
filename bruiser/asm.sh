#!/bin/bash
cd $(dirname $0)
clang ./asmrewriter.c -L./lua-5.3.4/src -llua -lm -ldl -lreadline -o asmrewriter 
./asmrewriter ./lua-scripts/asmtest.lua || exit 1
rm ./asmrewriter
