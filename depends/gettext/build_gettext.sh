#!/bin/bash
# usage ./build_gettext i686-w64-mingw32 or ./build_gettext x86_64-w64-mingw32
# libiconv
wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
tar xf libiconv-1.14.tar.gz
libiconvdir=`pwd`/libiconv-1.14
cd $libiconvdir
./configure --disable-shared --host="$1" --prefix="$libiconvdir/install_dir"
make install
cd ..

# gettext
wget http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.tar.gz
tar xf gettext-0.19.8.tar.gz
cd gettext-0.19.8
./configure --disable-shared --host="$1" --prefix=`pwd`/install_dir --with-libiconv-prefix="$libiconvdir/install_dir/"
make install
