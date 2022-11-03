#!/bin/bash

./basic $1.bas > $1.c
cat $1.c
gcc -g -o $1 $1.c && ./$1

