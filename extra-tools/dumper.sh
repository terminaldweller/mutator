#!/bin/bash

INPUT="./testFuncs1.c"
OUTPUT="./dump"
OPTIONS="--"
RUN="1"

while [[ $# -gt 0 ]]
do
	passarg="$1"

	case $passarg in
		-i|--input)
		INPUT="$2"
		shift
		;;
		-o|--output)
		OUTPUT="$2"
		shift
		;;
		-op|--options)
		OPTIONS="$2"
		shift
		;;
		-h|--help)
		echo "the script simply dumps the AST of the input."
		echo "-i|--input 	the input file full path."
		echo "-o|--output 	the output fule full path."
		echo "-op|--options 	extra options to pass to clang"
		echo "-h|--help 	prints out the help."
		RUN="0"
		;;
		*)
		echo $1 is not a valid argument...
		RUN="0"
		shift
	esac
	shift
done

if [[ "$RUN" == "1" ]]; then
	"clang" -Xclang -ast-dump -fsyntax-only "$INPUT" "$OPTIONS" > "$OUTPUT"

fi
