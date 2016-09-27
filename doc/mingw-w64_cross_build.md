# Cross compilation from Linux for windows

## Installing mingw-w64
```sh
$ sudo apt-get install gcc-mingw-w64 g++-mingw-w64
```

## Boost Libraries cross compilation with mingw-w64
__Download and extract tar.bz2 from:__ [boost_1_61_0.tar.bz2](http://www.boost.org/users/download/)
```sh
$ echo "using gcc : 4.9.2 : /usr/bin/x86_64-w64-mingw32-g++ ;" > user-config.jam
# echo "using gcc : : i686-w64-mingw32-g++ ;" > user-config.jam  -- for 32-bit version
$ ./bootstrap.sh --without-icu
# --prefix=/usr/i686-w64-mingw32/local  -- for 32-bit version
$ ./b2 -a --user-config=user-config.jam toolset=gcc target-os=windows variant=release \
> --prefix=/your/prefix/ threading=multi  threadapi=win32 link=shared \
> runtime-link=shared -j 2  --with-filesystem --with-system --with-program_options \
> -sNO_BZIP2=1 --sNO_ZLIB=1 --layout=tagged install
```

## Libsodium cross compilation with mingw-w64
__Download and extract tar.gz tarball from:__ [libsodium-1.0.11-mingw.tar.gz](https://download.libsodium.org/libsodium/releases/)

```sh
$ tar -xvzf libsodium-1.0.11.tar.gz
$ cd libsodium-1.0.11/
$ export CC=x86_64-w64-mingw32-gcc
$ ./autogen.sh
$ ./dist-build/msys2-win64.sh  # with commented make check (sed -i "s/make check && //" dist-build/msys2-win64.sh)
# includes and libraries should be build to local dir "libsodium-win64"
 ```

## Cross building galaxy42 with mingw-w64 dependencies:
__Go to galaxy42 main directory__
```sh
$ git clean -fdx
$ cmake -DBOOST_ROOT=/prefix/to/boost/mingw-w64/build \
> -DSODIUM_ROOT_DIR=/prefix/to/libsodium/mingw-w64/build \
> -DTARGET_ARCH=x86_64-w64-mingw32 -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake.in \
> -DCMAKE_BUILD_TYPE=Debug .
$ make tunserver.elf

```
After successful build you should see:
```sh
Linking CXX executable tunserver.elf.exe
[100%] Built target tunserver.elf
```

## Cross building NSIS installer:
__Cross build zlib__
```sh
$ git clone https://github.com/madler/zlib
$ cd zlib
$ git checkout v1.2.8
$ sed -e s/"PREFIX ="/"PREFIX = i586-mingw32msvc-"/ -i win32/Makefile.gcc
$ mv win32/Makefile.gcc Makefile
$ make
```
__Cross build NISI__
```sh
$ git clone https://github.com/kichik/nsis
$ cd nsis
$ git checkout f4c16d7eb827fdc256b57fbaba7be868fb779318
$ scons UNICODE=yes ZLIB_W32=../zlib PREFIX=install_dir install
$ scons UNICODE=yes SKIPUTILS=all SKIPMISC=all NSIS_CONFIG_CONST_DATA_PATH=no
$ PREFIX=install_dir install
$ cp ./Contrib/nsDialogs/nsDialogs.nsh Include/
$ cp ./build/urelease/UIs/modern.exe Contrib/UIs/
//$ save http://nsis.sourceforge.net/NSIS_Service_Lib plugin to Include/servicelib.nsh
$ mkdir -p Plugins/x86-ansi
$ mkdir -p Plugins/x86-unicode
$ cp `find | grep Plugins | grep x86-ansi | grep dll` Plugins/x86-ansi
$ cp `find | grep Plugins | grep x86-unicode | grep dll` Plugins/x86-unicode
$ cp -r install_dir/Stubs/ .
```
Copy `servicelib.nsh` from `galaxy42/contrib/nsis-installer-windows/plugins/NSIS_Service_Lib` to `Include` directory.

Now you can use NSIS to build installer
```sh
$ cd install_dir
$ ./makensis <nsis_script.nsi>
```
