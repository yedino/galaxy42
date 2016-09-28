#!/bin/bash

printf "\nWindows 32bit crossbuild script for Libsodium (with fixes)\n\n"

function fail() {
	printf "\nError in $0. (see above)\n\n"
	exit 1
}

export CFLAGS="-O3 -fomit-frame-pointer -m32 -march=pentium3 -mtune=westmere"
export PREFIX="$(pwd)/libsodium-win32"

if (i686-w64-mingw32-gcc --version > /dev/null 2>&1) then
  printf "\nMinGW found\n"
else
  printf "Please install mingw-w64-i686-gcc\n\n" >&2
  exit
fi

./configure PACKAGE_STRING='libsodium1.0.11' --prefix="$PREFIX" --exec-prefix="$PREFIX" \
            --host=i686-w64-mingw32 || fail
make clean || fail
make -j || fail
make install || fail

printf "\nWindows 32bit crossbuild script for Libsodium (with fixes) - DONE\n\n"
