#!/bin/bash

#the UI for mutator. it's supposed to mimic the normal 'nix argument passing.
#the arguments' positions are not important. you also get log and short options.

#default args
INPUT="./test/testFuncs1.c"
OUTPUT="./mutant.c"
OUTPUT_FORMAT="./mutant_format.c"
COMMAND="jack"

while [[ $# -gt 0 ]]
do
	passarg="$1"

	case $passarg in
		-c|--command)
		COMMAND="$2"
		shift
		;;
		-h|--help|-help)
		#COMMAND="$2"
		echo "-h, --help prints out the help.obviously..."
		echo "-c, --command you can specify the command you want to pass to mutator."
		echo "		clean runs make clean."
		echo "		build-all runs make all."
		echo "		run runs the mutator executable on the target."
		echo "		default runs build-all and then run."
		echo "		format calls clang-format to format the mutant. later to be used for the test command."
		echo "		test runs the tests on the executables and checks the results."
		echo "		misrac checks for misrac rules"
		echo "-v, --version prints out the version."
		echo "-i, --input, -input lets you choose the input file that is going to be passed to the mutator executable(s)."
		echo "-o, --output, -output lets you choose where to put the mutant."
		break
		;;
		-NDEBUG)
		echo "you are still deciding between passing the arg to make as a make arg or just writing into the makefile with the script."
		echo "btw youre running in NDEBUG dumbdumb! asserts are not gonna work like this."
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

if [[ "$COMMAND" == "clean" ]]; then
	echo "Running make clean..."
	echo "Killing all mutants..."
	"make" clean
	rm "$OUTPUT"
elif [[ "$COMMAND" == "format" ]]; then
	echo 'using clang-format to format the mutant...'
	"/home/bloodstalker/llvm-clang/build/bin/clang-format"	$OUTPUT > $OUTPUT_FORMAT
elif [[ "$COMMAND" == "test" ]]; then
	echo "you haven't implemented it yet..."
elif [[ "$COMMAND" == "build-all" ]]; then
	echo "Building all executables..."
	"make" all
elif [[ "$COMMAND" == "run" ]];then
	echo "Running executables on target file..."
	"./mutator" "$INPUT" -- > mutant-lvl1.c
	"./mutator-lvl2" mutant-lvl1.c -- > $OUTPUT
elif [[ "$COMMAND" == "misrac" ]]; then
	echo "checking input file(s) against Misra-C 2004..."
	"./mutator-lvl0" "$INPUT" -- > ./test/misra-log
elif [[ "$COMMAND" == "default" ]]; then
	echo "Building all target executables..."
	"make" all
	echo "Ruunning the input through clang-format..."
	"/home/bloodstalker/llvm-clang/build/bin/clang-format"	$INPUT -- > ./test/$OUTPUT_FORMAT
	"cp" ./test/$OUTPUT_FORMAT ./test/medium.c
	echo "Running all exetubales on target input..."
	echo "Level 1..."
	"./mutator" ./test/medium.c -- > ./test/mutant-lvl1.c
	echo "Level 2..."
	"./mutator-lvl2" ./test/mutant-lvl1.c -- > ./test/$OUTPUT
	echo 'Using clang-format to format the mutant...'
	"/home/bloodstalker/llvm-clang/build/bin/clang-format"	./test/$OUTPUT -- > ./test/$OUTPUT_FORMAT
elif [[ "$COMMAND" == "jack" ]]; then
	echo
else 
	echo "$COMMAND is not a valid command..."
	echo "Use --help for a list of valid commands..."
fi
