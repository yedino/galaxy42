#!/bin/bash -e

echo ""
echo "We now will build the NTru-Encrypt library"
echo "IN: the code of that lib should be downloaded with ./download with e.g. git submodules"
echo "OUT: if all works then the lib .so .a (or other on other OS) files are generated there in dir like .lib/"
echo ""

normaldir=$PWD

cd ../ntru-crypto/reference-code/C/Encrypt

echo "Building NTru - PLEASE WAIT..."
./autogen.sh && ./configure && make

cd "$normaldir"

mkdir -p build_extra/ntru/

cp -v -r ../ntru-crypto/reference-code/C/Encrypt/include/    build_extra/ntru

cp -v -r ../ntru-crypto/reference-code/C/Encrypt/.libs/    build_extra/ntru/

echo "Look at what we prepared:"

find build_extra/

echo "OK - All seems done in building external libraries" ; echo




