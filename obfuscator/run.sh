#!/bin/bash

#make
"./obfuscator" ./test/test.cpp
"./obfuscator" ./test/header.hpp --
#less dupe.cpp
