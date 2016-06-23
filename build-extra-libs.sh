#!/bin/bash -e

echo ""
echo "We now will build the NTru-Encrypt library"
echo "IN: the code of that lib should be downloaded with ./download with e.g. git submodules"
echo "OUT: if all works then the lib .so .a (or other on other OS) files are generated there in dir like .lib/"
echo ""

normaldir=$PWD

cd depends/ntru-crypto/reference-code/C/Encrypt

echo "Building NTru - PLEASE WAIT..."
./autogen.sh && ./configure && make

cd "$normaldir"

mkdir -p build_extra/ntru/

cp -v -r depends/ntru-crypto/reference-code/C/Encrypt/include/    build_extra/ntru/

cp -v -r depends/ntru-crypto/reference-code/C/Encrypt/.libs/    build_extra/ntru/


if [[ 1 ]] ;
then
	echo "Skipping step: Generating wisdom files (for e.g. NTru) - it is DISABLED in project configuration"
else
echo "Generating wisdom files (for e.g. NTru) ..."
cp -r depends/ntru-crypto/reference-code/C/Sign/PASS/data .
cp depends/ntru-crypto/reference-code/C/Sign/PASS/bin/wiseup.sh build_extra/
cd build_extra
for param in 433 577 769 1153
do
	./wiseup.sh $param
done
cd ..
fi

echo "Look at what we prepared:"

find build_extra/

echo "OK - All seems done in building external libraries" ; echo




