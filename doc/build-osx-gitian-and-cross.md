This is about crosscompilation for Mac OS X, that is:
you run the build process on: *Linux Ubuntu in Gitian* (though this instructions should work on other Linuxes too)
and you build program that will be used on: some *Mac OS X* (darwin).

This build process requires Apple SDK, which has some licencing limitations.
Check yourself can you use this files outside of an Apple device (e.g. other then running Linux in VM on Apple)
under the law system that applies to you.

Run all actions on Gitian as an user (default user of Gitian),
except for installing packages from repository - do this in Gitian as root.

Steps:
- get Apple SDK, as described in [../contrib/macdeploy/README_osx.md](readme about Mac deploy)
- start ./build-gitian -M image to have Gitian image
- build-gitian will unpack the SDK inside the Gitian (this is done automaticall)
- build-gitian will build the toolchain for crosscompilation inside the Gitian
	- get script from https://github.com/tpoechtrager/osxcross/blob/master/build.sh
	- copy SDK like: cp ~/build/MacOSX10.11.sdk.tar.gz ~/build/osxcross/tarballs/`
	- be in `~/build/osxcross/` and run `./build.sh`
	- now we will use toolchain with compiler: /home/ubuntu/build/osxcross/target/bin/x86_64-apple-darwin15-clang++
- patch boost library
	- in dir lib/locale/ apply contrib/gitian-patch/workaround-boost-12570.patch
- build boost library (see below)

Command to build boost that seems to work:

{{{

export WITH_ICONV="/home/ubuntu/build/macsdk/MacOSX10.11.sdk/usr/" &&  git clean -xdf ; git submodule foreach git clean -xdf; ./bootstrap.sh --without-icu &&   ./b2 headers  && export OSX_CPU_ARCH="core2" && export OSX_VERSION_MIN="10.8" &&  time   ./b2  --toolset=clang     --build-type=complete   --with-thread --with-locale    cxxflags=-mmacosx-version-min=${OSX_VERSION_MIN} cxxflags=-march=${OSX_CPU_ARCH} target-os=darwin architecture=x86 address-model=64  --layout=tagged  link=static runtime-link=static  -sNO_BZIP2=1 --sNO_ZLIB=1       --prefix=/home/ubuntu/build/boost/build-osx/   threading=multi    -sICONV_PATH="$WITH_ICONV"   ;  ls stage/lib/

}}}


# For other Linux, not running inside Gitian:

All the things that above are said to run inside Gitian, you can run on your other linux instead.

You need to install dependencies yourself if you do not use Gitian.
Some of them are listed in Gitian's .yml file, how ever the names of packages
are for Ubutnu and could be different on your OS.

Other source of deps to check is [https://github.com/tpoechtrager/osxcross/blob/master/tools/get_dependencies.sh] this includes:

```apt-get install -y clang llvm-dev libxml2-dev uuid-dev libssl-dev bash patch make tar xz-utils bzip2 gzip sed cpio```


# Work in progress

This almost works:

{{{

export WITH_ICONV="/home/ubuntu/build/macsdk/MacOSX10.11.sdk/usr/" &&  git clean -xdf ; git submodule foreach git clean -xdf; ./bootstrap.sh --without-icu &&   ./b2 headers  && export OSX_CPU_ARCH="core2" && export OSX_VERSION_MIN="10.8" &&  time   ./b2  --toolset=clang     --build-type=complete   --with-thread --with-locale    cxxflags=-mmacosx-version-min=${OSX_VERSION_MIN} cxxflags=-march=${OSX_CPU_ARCH} target-os=darwin architecture=x86 address-model=64  --layout=tagged  link=static runtime-link=static  -sNO_BZIP2=1 --sNO_ZLIB=1       --prefix=/home/ubuntu/build/boost/build-osx/   threading=multi  boost.locale.icu=off boost.locale.std=off    boost.locale.iconv=on    -sICONV_PATH="$WITH_ICONV"   ;  ls stage/lib/  

}}}


