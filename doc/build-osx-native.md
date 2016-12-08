This is about native Mac OS X build, that is:
you run the build process on: *Mac OS X* (darwin),
and you build program that will be used on: some *Mac OS X* (darwin).

Tested on: *OS X 10.11*.

1. Install homebrew:
	/usr/bin/ruby -e
"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

2. Install dependences:
	brew install git
	brew install autoconf
	brew install pkg-config
	brew install cmake
	brew install boost
	brew install libsodium
	brew install fftw

Interesting (AppleClang):
-- The C compiler identification is AppleClang 7.3.0.7030031
-- The CXX compiler identification is AppleClang 7.3.0.7030031

do script problems :

Building NTru - PLEASE WAIT...
Can't exec "aclocal": No such file or directory at /usr/local/Cellar/autoconf/2.69/share/autoconf/Autom4te/FileUtils.pm line 326.
autoreconf: failed to run aclocal: No such file or eirectory

cmake problems:

```
CMake Warning (dev):
  Policy CMP0042 is not set: MACOSX_RPATH is enabled by default.  Run "cmake
  --help-policy CMP0042" for policy details.  Use the cmake_policy command to
  set the policy and suppress this warning.

  MACOSX_RPATH is not specified for the following targets:

   sodiumpp
```

