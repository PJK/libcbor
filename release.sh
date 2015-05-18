#!/usr/bin/env bash
set -x

DIR=$(pwd) # Assume runing from this dir
OUTDIR=$(mktemp -d)
cd doc
make clean
cd ..
doxygen
cd doc
make html
cd build

cp -r html libcbor_docs_html
tar -zcf libcbor_docs.tar.gz libcbor_docs_html
rm -rf libcbor_docs_html

cp -r doxygen/html libcbor_api_docs_html
tar -zcf libcbor_api_docs.tar.gz libcbor_api_docs_html
rm -rf libcbor_api_docs_html

mv libcbor_docs.tar.gz libcbor_api_docs.tar.gz $OUTDIR

cd $OUTDIR

cmake $DIR -DCMAKE_BUILD_TYPE=Release
make
ctest
sudo make package

echo Relase ready in $OUTDIR
