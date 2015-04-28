#!/usr/bin/env bash
set -e

mkdir /tmp/build
cd /tmp/build
cmake -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON $SOURCE
make
ctest -V

if [ "$CC" == "gcc" ]; then
    make coverage
    coveralls-lcov coverage.info
fi

ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional\|Uninitialized Memory Read' memcheck.out; then
    exit 1
fi
