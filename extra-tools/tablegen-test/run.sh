#!/usr/bin/bash
cd $(dirname $0)
../luatablegen.py --tbg ../wasmtablegen.json --out ./ --luaheader ../../bruiser/lua-5.3.4/src --pre ./pre.txt --post ./post.txt
less ./jmp_s_t_tablegen.h
