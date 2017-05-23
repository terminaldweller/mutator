#!/bin/bash

#the UI for mutator. it's supposed to mimic the normal 'nix argument passing.
#the arguments' positions are not important. you also get long and short options.

#default args
#INPUT="./test/testFuncs1.c ./test/testFuncs2.c"
OUTPUT="./mutant.c"
OUTPUT_FORMAT="./mutant_format.c"
COMMAND="jack"
OPTIONS=""
COPTIONS=""
INPUT_FILE=""
PRINT_PRETTY=false

while [[ $# -gt 0 ]]
do
  passarg="$1"
  case $passarg in
    -c|--command)
    COMMAND="$2"
    shift
    ;;
    -h|--help|-help)
    echo "-h,   --help prints out the help.Obviously..."
    echo "-f, --file tells mutator to run the commands from the file."
    echo "-c,   --command you can specify the command you want to pass to mutator."
    echo "      clean runs make clean."
    echo "      build-all runs make all."
    echo "      run runs the mutator executable on the target."
    echo "      default runs build-all and then run."
    echo "      format calls clang-format to format the mutant. later to be used for the test command."
    echo "      test runs the tests on the executables and checks the results."
    echo "      misrac checks for misrac rules"
    echo "      auto-refac runs the automatic-refactoring on the inputs. auto-refac will run "
    echo "-v,   --version prints out the version."
    echo "-i,   --input, -input lets you choose the input file(or white-space-separated list of files) that is going to be passed to the mutator executable(s)."
    echo "-o,   --output, -output lets you choose where to put the mutant."
    echo "-pp,  --print-pretty, prints the output in a pretty format in a new file. The new file has the same name with a \"-pretty\" added to the name in the same directory."
    echo "-t, --test, runs the tests on the built executables. It should be followed by an executable name and the test to run on it. The accepted options are: tdd,valgrind,xsd,precommit."
    echo "  For example: ./mutator.sh --test mutator-lvl0 valgrind"
    echo "-opts   --options, pass options to the executable(s). The executables support all the clang options. Please enclose all the options in double quatation."
    echo "  For example: -opts \"-Wall std=c89\""
    echo "-copts  --customoptions, same as opts, but only used for custom options defined for the executable. For example: -copts \"-SysHeader=false -MainOnly=true\""
    echo ""
    echo "For a list of available options for each executable run them with -help to see a list."
    exit 0
    ;;
    -pp|--print-pretty)
    PRINT_PRETTY=true
    ;;
    -t|--test)
    if [[ "$2" == mutator-lvl0 && "$3" == valgrind ]]; then
      echo "Running command:"
      echo "valgrind ./"$2" -SysHeader=false -MainOnly=true  ./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c -- -std=c90 -I/lib/gcc/x86_64-redhat-linux/6.3.1/include > ./test/misra-log"
      "valgrind" ./"$2" -SysHeader=false -MainOnly=true  ./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c -- -std=c90 -I/lib/gcc/x86_64-redhat-linux/6.3.1/include > ./test/misra-log
    elif [[ "$2" == mutator-lvl0 && "$3" == tdd ]]; then
      echo "Running command:"
      echo "./mutator-lvl0 -SysHeader=false -MainOnly=true  ./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c -- -std=c90 -I/lib/gcc/x86_64-redhat-linux/6.3.1/include > ./test/misra-log"
      "./mutator-lvl0" -SysHeader=false -MainOnly=true  ./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c -- -std=c90 -I/lib/gcc/x86_64-redhat-linux/6.3.1/include > ./test/misra-log
    elif [[ "$2" == mutator-lvl0 && "$3" == xsd ]]; then
      echo "xmllint --noout --schema ./samples/mutator0-report-schema.xsd ./test/misrareport.xml"
      "xmllint" --noout --schema ./samples/mutator0-report-schema.xsd ./test/misrareport.xml
    elif [[ "$2" == precommit ]]; then
      cd ./extra-tools
      "./precommitTests.sh"
    else
      echo "unknown combination of options: $2 and $3"
      exit 127
    fi
    exit 0
    ;;
    -f|--file)
    INPUT_FILE="$2"
    while read -r line
    do
      case $line in 
        action_name:*)
        F_ACTION_NAME=${line:12:$((${#line}))}
        ;;
        executable_name:*)
        F_EXEC_NAME=${line:16:$((${#line}))}
        ;;
        exec_opts:*)
        F_EXEC_COPTS=${line:10:$((${#line}))}
        ;;
        in_files:*)
        F_IN_FILES=${line:9:$((${#line}))}
        ;;
        libtooling_options:*)
        F_LIBTOOLING_OPTS=${line:19:$((${#line}))}
        ;;
        out_files:*)
        F_OUT_FILE=${line:10:$((${#line}))}
        ;;
        log_files:*)
        F_LOG_FILE=${line:10:$((${#line}))}
        ;;
        \#*)
        #ignore.its a comment.
        ;;
        print_pretty:*)
        F_PRINT_PRETTY=${line:13:$((${#line}))}
        ;;
        end_action:*)
        F_END_ACTION=${line:11:$((${#line}))}
        if [[ "$F_END_ACTION" == run ]]; then
          echo "running $F_ACTION_NAME ..."
          echo "running ./$F_EXEC_NAME $F_EXEC_COPTS $F_IN_FILES -- $F_LIBTOOLING_OPTS > $F_OUT_FILE for $F_ACTION_NAME"
          if [[ "$F_EXEC_NAME" == safercpp-arr ]]; then
            eval "./safercpp/"$F_EXEC_NAME $F_EXEC_COPTS $F_IN_FILES -- $F_LIBTOOLING_OPTS > $F_OUT_FILE
          elif [[ "$F_EXEC_NAME" == bruiser ]]; then
            source "./bruiser/"$F_EXEC_NAME $F_EXEC_COPTS $F_IN_FILES -- $F_LIBTOOLING_OPTS
          else
            eval "./"$F_EXEC_NAME $F_EXEC_COPTS $F_IN_FILES -- $F_LIBTOOLING_OPTS > $F_OUT_FILE
          fi
          if [[ "$F_PRINT_PRETTY" = true ]]; then
            echo "running pretty print..."
            source ./extra-tools/ReportPrintPretty.sh $F_OUT_FILE $F_OUT_FILE-pretty
          fi
        elif [[ "$F_END_ACTION" == stop ]]; then
          #skip running ths action
          echo "skipping $F_ACTION_NAME ..."
        else
          echo "unknown option $F_END_ACTION for end_action. currently the only acceptable options are \"run\" and \"stop\"."
        fi
        F_ACTION_NAME=""
        F_EXEC_NAME=""
        F_EXEC_COPTS=""
        F_IN_FILES=""
        F_LIBTOOLING_OPTS=""
        F_OUT_FILE=""
        F_LOG_FILE=""
        F_PRINT_PRETTY=""
        F_END_ACTION=""
        ;;
      esac
    done < "$INPUT_FILE"
    exit 0
    shift
    ;;
    -v|--version)
    echo "mutator-lvl0 1.0.0 Pre-release"
    echo "mutator 1.0.0. Demo"
    exit 0
    ;;
    -i|--input|-input)
    while [[ ! "$2" == -* ]] && [[ $# -gt 0 ]]; do
      INPUT="$INPUT"" ""$2"
      shift
    done
    ;;
    -opts|--options)
    OPTIONS="$2"
    shift
    ;;
    -copts|--customoptions)
    COPTIONS="$2"
    shift
    ;;
    -o|--output|-output)
    OUTPUT="$2"
    shift
    ;;
    *)
    #not a valid argument
    echo  "$1 is not a valid argument..."
    exit 127
    ;;
  esac
  shift
done

if [[ "$COMMAND" == "clean" ]]; then
  echo "Running make clean..."
  echo "Killing all mutants..."
  "make" clean
  rm ./test/"$OUTPUT"
  rm ./test/misra-log
  rm ./test/medium.c
  rm ./test/mutant_format.c
  rm ./test/mutant-lvl1.c
  rm ./extra-tools/dump
elif [[ "$COMMAND" == "format" ]]; then
  echo 'using clang-format to format the mutant...'
  "/home/bloodstalker/llvm-clang/build/bin/clang-format"  $OUTPUT > $OUTPUT_FORMAT
elif [[ "$COMMAND" == "test" ]]; then
  echo "you haven't implemented it yet..."
elif [[ "$COMMAND" == "build-all" ]]; then
  echo "Building all executables..."
  "make" all
elif [[ "$COMMAND" == "run" ]];then
  echo "Running executables on target file..."
  "./mutator-lvl1" $INPUT -- > mutant-lvl1.c
  "./mutator-lvl2" mutant-lvl1.c -- > $OUTPUT
elif [[ "$COMMAND" == "misrac" ]]; then
  echo "Removing previous XML report..."
  "rm" ./test/misrareport.xml
  echo "Removing previous JSON report..."
  "rm" ./test/misrareport.json
  echo "checking input file(s) against Misra-C 2004..."
  echo "Command to run:"
  echo "./mutator-lvl0 ${COPTIONS:0:$((${#COPTIONS}))} $INPUT -- ${OPTIONS:0:$((${#OPTIONS}))} > ./test/misra-log"
  "./mutator-lvl0" ${COPTIONS:0:$((${#COPTIONS}))} $INPUT -- ${OPTIONS:0:$((${#OPTIONS}))} > ./test/misra-log
  if [[ "$PRINT_PRETTY" = true ]]; then
    echo "running pretty print..."
    source ./extra-tools/ReportPrintPretty.sh ./test/misra-log ./test/misra-log-pretty
  fi
elif [[ "$COMMAND" == "default" ]]; then
  echo "This option is deprecated..."
  exit 0
  echo "Building all target executables..."
  "make" all CXX=clang++ BUILD_MODE=COV_NO_CLANG_1Z
  echo "Ruunning the input through clang-format..."
  "clang-format"  $INPUT -- > ./test/$OUTPUT_FORMAT
  "cp" ./test/$OUTPUT_FORMAT ./test/medium.c
  echo "Running all exetubales on target input..."
  echo "Level 1..."
  "./mutator-lvl1" ./test/medium.c -- > ./test/mutant-lvl1.c
  echo "Level 2..."
  "./mutator-lvl2" ./test/mutant-lvl1.c -- > ./test/$OUTPUT
  echo "Using clang-format to format the mutant..."
  "clang-format"  ./test/$OUTPUT -- > ./test/$OUTPUT_FORMAT
elif [[ "$COMMAND" == "auto-refac" ]]; then
  echo "Warning! This option is not yet completely implemented."
  echo "Copying input file(s) to temp..."
  CP_INPUT_ARR=$(echo "$INPUT" | awk 'BEGIN{RS=" "}{print $1}')
  echo $CP_INPUT_ARR
  "cp" $CP_INPUT_ARR ./temp
  echo "Running clang-format on the copies of input(s)..."
  RUN_INPUT_ARR=$(ls ./temp | gawk 'BEGIN{RS="\n"}{print $1}')
  for loopinput in ${RUN_INPUT_ARR[@]}; do
    echo mut1-$loopinput
    echo "Running automatic-refactoring on input mut1-$loopinput..."
    "./mutator-lvl1" ./temp/$loopinput -- -w > ./temp/mut1-$loopinput
    "clang-format" -i $loopinput
    "./mutator-lvl2" ./temp/mut1-$loopinput -- -w > ./temp/mut2-$loopinput
    "clang-format" -i ./temp/mut2-$loopinput
  done
elif [[ "$COMMAND" == "jack" ]]; then
  echo
else 
  echo "$COMMAND is not a valid command..."
  echo "Use --help for a list of valid commands..."
  exit 1
fi
