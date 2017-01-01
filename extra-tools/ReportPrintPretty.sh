#!/bin/bash
#the first input should be the raw report output
#the second output should be the file name for the pretty output
"cat" "$1" | gawk '
BEGIN{RS = "\n";FS = ":"}
function red(s) {printf "\033[1;31m" s "\033[0m "}
function green(s) {printf "\033[1;32m" s "\033[0m "}
function blue(s) {printf "\033[1;34m" s "\033[0m "}
	{print red($1)green($2)green($3)blue($4)blue($5)}
END{}
' > "$2"