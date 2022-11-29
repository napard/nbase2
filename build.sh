#!/bin/bash

gcc -Wall -g -pedantic \
    -DNBASE_DEFINITIONS \
    -DNBASE_IMPLEMENTATION \
    -DNBASE_DEFINE_PROPER_MAIN \
    -DNBASE_DEFINE_ATEXIT \
    src/core/main.c -o nbase -lm



