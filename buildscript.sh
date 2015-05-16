#!/usr/bin/env bash
set -e

cppcheck $SOURCE --error-exitcode=1
cmake $SOURCE -DCUSTOM_ALLOC=$CUSTOM_ALLOC -DCMAKE_BUILD_TYPE=Debug
make VERBOSE=1
ctest -V

ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional\|Uninitialized Memory Read' memcheck.out; then
    exit 1
fi
