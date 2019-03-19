#!/bin/sh
cd $(dirname $0)
"../../faultreiber/faultreiber.py" --name wasm --outdir ./ft/ --datetime --xml ./wasm.xml --calloc --voidtraininitsize 60 --voidtrainfactor 1.9
#"../../faultreiber/faultreiber.py" --name wasm --outdir ./ft/ --datetime --xml ./wasm.xml --luaalloc --voidtraininitsize 60 --voidtrainfactor 1.9 --luaheaders ../../../lua-5.3.4/src
"clang-format" -i ./ft/read.c ./ft/structs.c ./ft/structs.h ./ft/aggregate.c ./ft/aggregate.h ./ft/read.h
#"less" ./test/structs.h
