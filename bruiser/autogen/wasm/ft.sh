#!/bin/sh
cd $(dirname $0)
"../../faultreiber/faultreiber.py" --name wasm --outdir ./ft/ --datetime --xml ./ft/wasm.xml --calloc
#"clang-format" -i ./test/read.c ./test/structs.c ./test/structs.h ./test/aggregate.c ./test/aggregate.h ./test/read.h
#"less" ./test/structs.h
