#!/bin/bash
cd $(dirname $0)
gcc wasm.c -o wasme -L ./lua-5.3.4/src/ -llua -lm -ldl
./wasme || exit $?
rm ./wasme


