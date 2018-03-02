#!/bin/bash
cd $(dirname $0)
clang ./asmrewriter.c -llua -o asmrewriter
./asmrewriter ./lua-scripts/asmtest.lua || exit 1
rm ./asmrewriter
