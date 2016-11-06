#!/usr/bin/env bash
set -e

PREVD=$(pwd)
cd $SOURCE
cppcheck . --error-exitcode=1 --suppressions cppcheck_suppressions.txt --force
cd $PREVD

cmake $SOURCE -DCBOR_CUSTOM_ALLOC=ON -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON -DCMAKE_PREFIX_PATH=$HOME/usr/local
make VERBOSE=1
./test/fuzz_test

ctest -VV

ctest -T memcheck | tee memcheck.out
if grep -q 'Memory Leak\|IPW\|Uninitialized Memory Conditional\|Uninitialized Memory Read' memcheck.out; then
    exit 1
fi
