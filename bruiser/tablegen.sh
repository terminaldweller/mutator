#!/usr/bin/bash
cd $(dirname $0)
../extra-tools/luatablegen.py --tbg ./wasmtablegen.json --out ./luatablegen --luaheader ../lua-5.3.4/src --pre ./luatablegen/wasmheader.txt --headeraggr ./luatablegen/wasm_tables.h --lualibpath ./lua-scripts/wasm.lua --docpath /home/bloodstalker/extra/mutator.wiki/wasm.md
if [[ $1 == test ]]; then
  make -C ./luatablegen
  make clean
fi
