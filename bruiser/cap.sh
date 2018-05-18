#!/bin/bash
cd $(dirname $0)
clang -c ./asmrewriter.c -o asmrewriter.o
clang ./bruisercapstone.c -L/usr/local/lib64 -lcapstone -lkeystone -llua asmrewriter.o -o bcapstone
./bcapstone || exit 1
rm ./bcapstone
