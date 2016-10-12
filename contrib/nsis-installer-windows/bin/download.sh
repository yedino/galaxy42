#!/bin/bash

function fail() {
	echo "Error (in $0) : " "$@"
	exit 1
}

echo "Script $0 (in PWD=$PWD)"
cat <<EOF

This script will 'download' or obtain the DLL files.
They are needed during cross-building for Windows (from Linux) - this should be done inside of Gitian.

Files are taken from system, so are obtained with apt-get, we trust security of that mechanism here.

(But anyway the .dll file will show up on some checksum raports and ofcourse are in the main installer)

Some command to test what dlls are needed, and it result (as example) is:

 for dll in libwinpthread-1.dll libstdc++-6.dll libgcc_s_sjlj-1.dll libgcc_s_seh-1.dll ; do echo "dll=$dll" ; apt-file search "$dll" ; echo ; done


 dll=libwinpthread-1.dll
 mingw-w64-i686-dev: /usr/i686-w64-mingw32/lib/libwinpthread-1.dll
 mingw-w64-x86-64-dev: /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll

 dll=libstdc++-6.dll
 g++-mingw-w64-i686: /usr/lib/gcc/i686-w64-mingw32/5.3-posix/libstdc++-6.dll
 g++-mingw-w64-i686: /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libstdc++-6.dll
 g++-mingw-w64-x86-64: /usr/lib/gcc/x86_64-w64-mingw32/5.3-posix/libstdc++-6.dll
 g++-mingw-w64-x86-64: /usr/lib/gcc/x86_64-w64-mingw32/5.3-win32/libstdc++-6.dll

 dll=libgcc_s_sjlj-1.dll
 gcc-mingw-w64-i686: /usr/lib/gcc/i686-w64-mingw32/5.3-posix/libgcc_s_sjlj-1.dll
 gcc-mingw-w64-i686: /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libgcc_s_sjlj-1.dll

 dll=libgcc_s_seh-1.dll
 gcc-mingw-w64-x86-64: /usr/lib/gcc/x86_64-w64-mingw32/5.3-posix/libgcc_s_seh-1.dll
 gcc-mingw-w64-x86-64: /usr/lib/gcc/x86_64-w64-mingw32/5.3-win32/libgcc_s_seh-1.dll


EOF

function takedll() {
	cp -v "$2" "$1/" || fail "Can not copy file from $2"
}

takedll x86 /usr/i686-w64-mingw32/lib/libwinpthread-1.dll
takedll x64 /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll

# full posix
takedll x86 /usr/lib/gcc/i686-w64-mingw32/5.3-posix/libstdc++-6.dll
takedll x64 /usr/lib/gcc/x86_64-w64-mingw32/5.3-posix/libstdc++-6.dll
takedll x86 /usr/lib/gcc/i686-w64-mingw32/5.3-posix/libgcc_s_sjlj-1.dll
takedll x64 /usr/lib/gcc/x86_64-w64-mingw32/5.3-posix/libgcc_s_seh-1.dll

# only win32
#takedll x86 /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libstdc++-6.dll
#takedll x64 /usr/lib/gcc/x86_64-w64-mingw32/5.3-win32/libstdc++-6.dll
#takedll x86 /usr/lib/gcc/i686-w64-mingw32/5.3-win32/libgcc_s_sjlj-1.dll
#takedll x64 /usr/lib/gcc/x86_64-w64-mingw32/5.3-win32/libgcc_s_seh-1.dll


