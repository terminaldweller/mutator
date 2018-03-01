#!/bin/bash
cd $(dirname $0)
clang ./asmrewriter.c -llua -o asmrewriter
./asmrewriter || exit 1
rm ./asmrewriter
