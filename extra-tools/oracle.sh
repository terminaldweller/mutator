#!/bin/bash
Red="\033[0;31m"
Green="\033[0;32m"
Lblue="\033[1;34m"
Orange="\033[0;33m"
Magenta="\033[1;35m"
NC="\033[0m"
########################################################################################################################
printf "${Magenta}Running oracle...\n${NC}" | tee -a $1

#m0
"diff" ./test/misrareport.xml ./oracle/m0/oracle-m0.xml | tee ./oralce-m0-diff

printf "${Magenta}Running oracle for m0...\n${NC}" | tee -a $1
if [ -s oracle-m0-diff ]; then
  printf "${RED}The oracle test failed.\n${NC}" | tee -a $1
fi

printf "${Green}The oracle test passed.\n${NC}" | tee -a $1
