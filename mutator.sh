#!/bin/bash

#the UI for mutator. it's supposed to mimic the normal 'nix argument passing.
#the arguments' positions are not important. you also get log ans short options.

#default args
INPUT="./covtest/testFuncs1.c"
OUTPUT="./mutant.c"
COMMAND="all"

while [[ $# -gt 0 ]]
do
	passarg="$1"

	case $passarg in
		-c|--command)
		COMMAND="$2"
		shift
		;;
		-h|--help)
		COMMAND="$2"
		echo "Currently there is no help for this."
		break
		;;
		-v|--version)
		echo "Version 1.0.0"
		break
		;;
		-i|--input|-input)
		INPUT="$2"
		shift
		;;
		-o|--output|-output)
		OUTPUT="$2"
		shift
		;;
		*)
		#not a valid argument
		echo  "$1 $2 is not a valid argument..."
		break
		;;
	esac
	shift
done

if [[ "$COMMAND" == clean ]]; then
	echo "Running make clean..."
	echo "Killing all mutants..."
	"make" clean
	rm "$OUTPUT"
elif [[ "$COMMAND" == build-all ]]; then
	echo "Building all executables..."
	"make" all
elif [[ "$COMMAND" == run ]];then
	echo "Running executables on target file..."
	"./mutator" "$INPUT" -- > "$OUTPUT"
elif [[ "$COMMAND" == "default" ]]; then
	echo "Building all target executables..."
	"make" all
	echo "Running all exetubales on target input..."
	"./mutator" "$INPUT" -- > "$OUTPUT"
else 
	echo "$COMMAND is not a valid command..."
fi
