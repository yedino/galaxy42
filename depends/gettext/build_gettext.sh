#!/bin/bash

# libiconv
wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
tar xf libiconv-1.14.tar.gz
libiconvdir=`pwd`/libiconv-1.14
cd $libiconvdir
./configure --disable-shared --host=i686-w64-mingw32 --prefix="$libiconvdir/install_dir"
make install
cd ..
#echo "******************$libiconvdir/install_dir/"
#exit 1
# gettext
wget http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.8.tar.gz
tar xf gettext-0.19.8.tar.gz
cd gettext-0.19.8
./configure --disable-shared --host=i686-w64-mingw32 --prefix=`pwd`/install_dir --with-libiconv-prefix="$libiconvdir/install_dir/"
make install
