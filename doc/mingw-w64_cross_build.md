# Cross compilation from Linux for windows

## Installing mingw-w64
```sh
    sudo apt-get install gcc-mingw-w64 g++-mingw-w64  
```

## Boost Libraries compilation for mingw-w64
__download from:__ [boost_1_61_0.tar.bz2](http://www.boost.org/users/download/)
```sh
$ echo "using gcc : 4.9.2 : /usr/bin/x86_64-w64-mingw32-g++ ;" >> user-config.jam  
# echo "using gcc : : i686-w64-mingw32-g++ ;" > user-config.jam  -- for 32-bit version
$ ./bootstrap.sh --with-libraries=system,filesystem,program_options --prefix=/usr/x84_64-w64-mingw32/local  
# --prefix=/usr/i686-w64-mingw32/local  -- for 32-bit version  
$ ./b2 -j2 --debug-configuration  toolset=gcc --user-config=user-config.jam target-os=windows
$ ./bjam
$ sudo ./bjam install
```




## Libsodium compilation for mingw-w64
__download tarball from:__ [libsodium-1.0.11-mingw.tar.gz](https://download.libsodium.org/libsodium/releases/)

```sh
$ tar -xvzf libsodium-1.0.11.tar.gz
$ cd libsodium-1.0.11/
$ export CC=x86_64-w64-mingw32-gcc
$ ./dist-build/msys2-win64.sh  # with commented make check
 ```
 
 ## Include and linking mingw dependencies (non-pretty):  
__CMakeLists.txt:__ 
```include_directories("/usr/x84_64-w64-mingw32/local/include")```
