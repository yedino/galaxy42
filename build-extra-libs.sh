#!/bin/bash -e

echo ""
echo "We now will build the NTru-Encrypt library"
echo "IN: the code of that lib should be downloaded with ./download with e.g. git submodules"
echo "OUT: if all works then the lib .so .a (or other on other OS) files are generated there in dir like .lib/"
echo ""

[[ -z "$COVERAGE" ]] && { echo "set env variable COVERAGE" ; exit 1 ; }
[[ -z "$EXTLEVEL" ]] && { echo "set env variable EXTLEVEL" ; exit 1 ; }

DIR_BUILDEXTRA="build_extra"
mkdir -p "$DIR_BUILDEXTRA"
[ -d "$DIR_BUILDEXTRA" ] || { echo "Failed to create dir DIR_BUILDEXTRA=$DIR_BUILDEXTRA." ; exit 1; }
[ -w "$DIR_BUILDEXTRA" ] || { echo "Failed to create (not writable!) dir DIR_BUILDEXTRA=$DIR_BUILDEXTRA." ; exit 1; }

normaldir=$PWD


if (( EXTLEVEL >= 20 )) # {{match_extlevel_ntru}}
then
	echo "Building NTru (Encrypt) - PLEASE WAIT..."
	pushd depends/ntru-crypto/reference-code/C/Encrypt
	./autogen.sh && ./configure && make
	popd

	mkdir -p build_extra/ntru/

	cp -v -r depends/ntru-crypto/reference-code/C/Encrypt/include/    build_extra/ntru/

	cp -v -r depends/ntru-crypto/reference-code/C/Encrypt/.libs/    build_extra/ntru/


	if [[ 0 ]] ; then
		echo "Skipping step: Generating wisdom files (for e.g. NTru) - it is DISABLED in project configuration"
	else
		echo "Generating wisdom files (for e.g. NTru) ..."
		cp -r depends/ntru-crypto/reference-code/C/Sign/PASS/data .
		cp depends/ntru-crypto/reference-code/C/Sign/PASS/bin/wiseup.sh build_extra/
		pushd build_extra
		for param in 433 577 769 1153
		do
			./wiseup.sh $param
		done
		popd
	fi
	echo "This is one part of NTru (Encrypt), other part could be elsewhere e.g. in CMake"
fi

echo "Look at what we prepared:"

find build_extra/

echo "OK - All seems done in building external libraries here (there could be more of this in other parts of build process, e.g. in the main CMake calling sub CMakes)" ; echo




