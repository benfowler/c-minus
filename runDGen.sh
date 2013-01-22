#!/bin/bash

./compiler -c -f $1
dgen output.dcl
gcc -o output output.s misc/stdio.c
