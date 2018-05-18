#!/bin/bash
cd $(dirname $0)
gcc ./bruiserffi.c -lffi -lcapstone -o ffi
./ffi
rm ./ffi
