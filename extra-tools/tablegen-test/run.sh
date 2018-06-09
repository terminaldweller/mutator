#!/usr/bin/bash
cd $(dirname $0)
../luatablegen.py --tbg ../wasmtablegen.json --out ../../bruiser/luatablegen --luaheader ../../bruiser/lua-5.3.4/src --pre ./wasmheader.txt --headeraggr ../../bruiser/luatablegen/wasm_tables.h --lualibpath ../../bruiser/lua-scripts/wasm.lua
#../luatablegen.py --tbg ../wasmtablegen.json --out ../../bruiser/luatablegen --luaheader ../../bruiser/lua-5.3.4/src --pre ./wasmheader.txt --singlefile --outfile ../../bruiser/luatablegen/wasmtablegen.h
for filename in ../../bruiser/luatablegen/*.c; do
  gcc -c $filename > /dev/null 2>&1
  if [[ $? != 0 ]]; then
    echo $filename did not compile.
  fi
done
