#! /bin/sh

echo "Windows crossbuild script for Libsodium (with fixes)"

export CFLAGS="-O3 -fomit-frame-pointer -m64 -mtune=westmere"
export PREFIX="$(pwd)/libsodium-win64"

if (x86_64-w64-mingw32-gcc --version > /dev/null 2>&1) then
  echo MinGW found
else
  echo Please install mingw-w64-x86_64-gcc >&2
  exit
fi

./configure PACKAGE_STRING='libsodium1.0.11' --prefix="$PREFIX" --exec-prefix="$PREFIX" \
            --host=x86_64-w64-mingw32 && \
make clean && \
make -j && \
\
make install
