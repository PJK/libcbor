#!/usr/bin/env bash
set -e

mkdir /tmp/build
cd /tmp/build
cmake -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug $SOURCE
make
ctest -V
ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional' memcheck.out; then
    exit 1
fi

lcov --compat-libtool --directory . --capture --output-file coverage.info
coveralls-lcov coverage.info
