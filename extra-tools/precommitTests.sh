#!/bin/bash

Red="\033[0;31m"
Green="\033[0;32m"
Lblue="\033[1;34m"
NC="\033[0m"

printf "${Lblue}switching to mutator root...\n${NC}"
cd ..

"make" clean
echo ""

printf "${Lblue}testing the build in C++11...\n${NC}"
"make" CXX=clang++ BIULD_MODE=COV_NO_CLANG
echo ""

if [[ $? == 0 ]]; then
  printf "${Green}mutator c++11 test build passed.\n${NC}"
else
  printf "${Red}mutator c++11 test build failed.\n${NC}"
fi

# running mutator-lvl0 on the tdd sources
"./mutator-lvl0" -SysHeader=false -MainOnly=true ./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c -- -std=c90 -I/lib/gcc/x86_64-redhat-linux/6.3.1/include -Wall > ./test/misra-log

if [[ $? == 0 ]]; then
  echo "${Green}mutator-lvl0 smoke test on the tdds passed.\n${NC}"
else
  echo "${Red}mutator-lvl0 smoke test on the tdds failed.\n${NC}"
fi



