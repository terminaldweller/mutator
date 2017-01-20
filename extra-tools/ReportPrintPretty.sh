#!/bin/bash
#the first input should be the raw report output
#the second output should be the file name for the pretty output
#function red(string) { printf ("%s%s%s", "\033[1;31m", string, "\033[0m "); }
"cat" "$1" | gawk '
BEGIN{RS = "\n";FS = ":"}
function red(s) {printf "\033[1;31m" s "\033[0m "}
function green(s) {printf "\033[1;32m" s "\033[0m "}
function yellow(s) {printf "\033[1;33m" s "\033[0m "}
function blue(s) {printf "\033[1;34m" s "\033[0m "}
function purple(s) {printf "\033[1;35m" s "\033[0m "}
function cyan(s) {printf "\033[1;36m" s "\033[0m "}
function unred(s) {printf "\033[1;41m" s "\033[0m "}
function ungreen(s) {printf "\033[1;42m" s "\033[0m "}
function unyellow(s) {printf "\033[1;43m" s "\033[0m "}
	{print red($1)green($2)purple($3)blue($4)blue($5)yellow($6)cyan($7)ungreen($8)ungreen($9)}
END{}
' > "$2"