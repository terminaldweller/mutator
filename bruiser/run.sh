#!/bin/bash

cd $(dirname $0)
#"./bruiser" --verbose --lua ./lua-scripts/demo1.lua
"./bruiser" --lua ./lua-scripts/demo1.lua
#"./bruiser"  ../test/bruisertest/test.cpp --src --verbose --lua ./lua-scripts/demo1.lua
#gdb "./bruiser ../test/bruisertest/test.cpp --src"
