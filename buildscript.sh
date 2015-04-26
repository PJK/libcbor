#!/usr/bin/env bash
set -e

mkdir /tmp/build
cd /tmp/build
cmake -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug $SOURCE
make
ctest -V
ctest -T memcheck | tee memcheck.out
grep -vE 'Memory Leak - [1-9][0-9]*' memcheck.out
