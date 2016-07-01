OS X 10.11

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

CMake Warning (dev):
  Policy CMP0042 is not set: MACOSX_RPATH is enabled by default.  Run "cmake
  --help-policy CMP0042" for policy details.  Use the cmake_policy command to
  set the policy and suppress this warning.

  MACOSX_RPATH is not specified for the following targets:

   sodiumpp

make :

galaxy42/depends/sidh/SIDH_setup.c:13:10: fatal error: 'malloc.h' file not found
#include <malloc.h>
				39c16dc6784c3df611cab1dd62391b917f6cef85


*On mac, build_extra libs don't create include directory. 
change ntru includes example:
	#include "ntru/include/ntru_crypto_drbg.h"
to:
	#include "ntru/ntru_crypto_drbg.h"
