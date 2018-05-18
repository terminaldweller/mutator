#!/bin/sh

"gcc" ramdump.c -o ramdump
PID=$("pgrep" Dwarf_Fortress)
"./ramdump" $PID
echo
"ls" -s /tmp/ramdump
