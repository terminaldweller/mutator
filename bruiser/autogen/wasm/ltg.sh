#!/bin/sh
cd $(dirname $0)
"../../tablegen/luatablegen.py" --out ./ltg/ --luaheader ../../../lua-5.3.4/src --headeraggr ./ltg/wasm_tables.h --lualibpath ./ltg/wasm.lua --docpath ./ltg/wasm.md --xml ./ltg/luwasm.xml --tbldefs ./ltg/ --name wasm --anon
clang-format ./ltg/*.c ./ltg/*.h -i
for filename in ./ltg/*.c; do
  gcc -c $filename > /dev/null 2>&1
  if [[ $? != 0 ]]; then
    echo $filename did not compile.
  fi
done
rm *.o
