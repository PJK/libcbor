#!/usr/bin/env bash
set -e

if [ "$CC" == "gcc" ]; then
    cmake $SOURCE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCOVERAGE=ON -DCUSTOM_ALLOC=$CUSTOM_ALLOC
else
    cmake -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON $SOURCE
fi

make
ctest -V

if [ "$CC" == "gcc" ]; then
    lcov --capture --directory $(pwd) --output-file coverage.info
    coveralls-lcov coverage.info
fi

ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional\|Uninitialized Memory Read' memcheck.out; then
    exit 1
fi
