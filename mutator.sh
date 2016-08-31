#!/bin/bash

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
		;;
		-v|--version)
		echo "Version 1.0.0"
		break
		;;
		-t|--target)
		INPUT="$2"
		shift
		;;
		*)
		#not a valid argument
		echo  "$2 is not a valid argument."
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
	"$OUTPUT" "$INPUT" --
elif [[ "$COMMAND" == "default" ]]; then
	echo "Building all target executables."		
	echo "Running all exetubales on target input."
	"make" all
	"./mutator" "$INPUT" -- > "$OUTPUT"
fi
