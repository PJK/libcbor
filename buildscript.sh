#!/usr/bin/env bash
set -e

mkdir /tmp/build
cd /tmp/build
cmake -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON $SOURCE
make
ctest -V

if [ "$CC" == "gcc" ]; then
    # For some reason, this needs clean build
    rm -rf *
    cmake $SOURCE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCOVERAGE=ON -DCUSTOM_ALLOC=$CUSTOM_ALLOC
    make
    ctest
    lcov --capture --directory . --output-file coverage.info
    coveralls-lcov coverage.info
fi

ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional\|Uninitialized Memory Read' memcheck.out; then
    exit 1
fi
