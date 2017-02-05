#!/usr/bin/env bash
set -x
set -e

DIR=$(pwd) # Assume runing from this dir
OUTDIR=$(mktemp -d)
pushd doc
make clean
popd
doxygen
cd doc
make html
cd build

cp -r html libcbor_docs_html
tar -zcf libcbor_docs.tar.gz libcbor_docs_html

cp -r doxygen/html libcbor_api_docs_html
cp -r doxygen/html $DIR/docs/doxygen
tar -zcf libcbor_api_docs.tar.gz libcbor_api_docs_html

mv libcbor_docs.tar.gz libcbor_api_docs.tar.gz $OUTDIR

cd $OUTDIR

#cmake $DIR -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON
#make
#ctest

echo Relase ready in $OUTDIR
