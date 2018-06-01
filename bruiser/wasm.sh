#!/bin/bash
cd $(dirname $0)
gcc wasm.c -o wasme
./wasme
rm ./wasme


