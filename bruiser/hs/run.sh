#!/bin/bash

ghc -c -O Safe.hs
ghc --make -no-hs-main -optc-O bruiserhs.c Safe -o bruiserhs

"./bruiserhs"
