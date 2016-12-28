#!/bin/bash

printf "\nclang apple-darwin15 osx 64bit crossbuild script for Libsodium (with fixes)\n\n"

function fail() {
	printf "\nError in $0. (see above)\n\n"
	exit 1
}

export PREFIX="$(pwd)/libsodium-osx"
export OSX_VERSION_MIN=${OSX_VERSION_MIN-"10.8"}
export OSX_CPU_ARCH=${OSX_CPU_ARCH-"core2"}

mkdir -p $PREFIX || fail

export CFLAGS="-arch x86_64 -mmacosx-version-min=${OSX_VERSION_MIN} -march=${OSX_CPU_ARCH} -O2 -g -flto"
export LDFLAGS="-arch x86_64 -mmacosx-version-min=${OSX_VERSION_MIN} -march=${OSX_CPU_ARCH} -flto"

make distclean > /dev/null

./configure --enable-minimal \
            --prefix="$PREFIX" \
            --host=x86_64-apple-darwin15 || fail

make -j3 install || fail

# Cleanup
make distclean > /dev/null

printf "\nclang apple-darwin11 osx 64bit crossbuild script for Libsodium (with fixes) - DONE\n\n"
