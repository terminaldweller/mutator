#!/bin/bash

cd $(dirname $0)
"make"
#"./bruiser"
"./bruiser"  ../test/bruisertest/test.cpp --src
#gdb "./bruiser ../test/bruisertest/test.cpp --src"
