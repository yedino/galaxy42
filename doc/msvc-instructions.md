# GALAXY42 compilation on msvc (Visual Studio 2015):

### Process instructions for:

- version:
	```commit ca9aefcdc45f2b4dc76fd83e5de9a22636459ed9```
	- It should be similar or easier for newer versions

### Used tools:
- Cygwin (for git)
    - could be other tool that supports git
- Visual Studio Community 2015
- Cmake-GUI 3.6.0

### Cygwin:
```sh
$ git clone https://github.com/yedino/galaxy42
$ echo "REMEMBER TO VERIFY CHECKSUM (git log -1) - git revision hash" ; read _
$ cd galaxy42
$ git submodule update --init
```

### MSVC:
Creating project for Visual Studo (.sln and .vcxproj's) using Cmake-GUI

__Provide dependencies (libboost and libsodium):__
- Pre-compiled binaries can be found on libboost/libsodium official pages.
Download and install/unpack libraries. I used:
    - [Libsodium Releases](https://download.libsodium.org/libsodium/releases/)
    - [Prebuild Boost Binaries For Windows v.1.6.1](https://sourceforge.net/projects/boost/files/boost-binaries/1.61.0/)

__Open CMake-gui:__
- __Settings:__
	- Where is the source: 		path/to/cloned/galaxy42
	- Where to build the binaries:	any/output/path

__Set cmake variables:__

- Click "Add Entry"
- "Name": BOOST_ROOT, BOOST_LIBRARYDIR, SODIUM_ROOT_DIR
- "Type": PATH
- "Value":
	- for SODIUM_ROOT_DIR: full path to directory, where you install/unpack "Libsodium Releases"
	- for BOOST_ROOT: full path to directory, where you install/unpack "Prebuild Boost Binaries For Windows v.1.6.1"
	- for BOOST_LIBRARYDIR: go to directory, where you install/unpack "Prebuild Boost Binaries For Windows v.1.6.1", open folder (lib32-msvc-14.0/lib64-msvc-14.0 or similar) complies with your system version (x32 or x64) and copy path

```
click configure
* for 32-bit build choose Visual Studio 14 2015
* for 64-bit build choose Visual Studio 14 2015 Win64
click generate
```
__Cmake output:__
```
	The C compiler identification is MSVC 18.0.40629.0
	The CXX compiler identification is MSVC 18.0.40629.0
	Check for working C compiler: C:/Program Files/Microsoft Visual Studio 12.0/VC/bin/cl.exe
	Check for working C compiler: C:/Program Files/Microsoft Visual Studio 12.0/VC/bin/cl.exe -- works
	Detecting C compiler ABI info
	Detecting C compiler ABI info - done
	Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio 12.0/VC/bin/cl.exe
	Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio 12.0/VC/bin/cl.exe -- works
	Detecting CXX compiler ABI info
	Detecting CXX compiler ABI info - done
	Detecting CXX compile features
	Detecting CXX compile features - done
	CMAKE_BUILD_TYPE = debug
	EXTLEVEL = 0
	COVERAGE = OFF
	EXTLEVEL enabling EXTLEVEL_IS_NORMAL
	Could NOT find PythonInterp (missing:  PYTHON_EXECUTABLE)
	Looking for pthread.h
	Looking for pthread.h - not found
	Found Threads: TRUE
	JsonCpp Version: 1.7.2
	Could NOT find PythonInterp (missing:  PYTHON_EXECUTABLE) (Required is at least version "2.6")
	Disabling COVERAGE
	Debug build
	Libs for crypto are: (as string) ''
	Libs for crypto are: (as list) ''
	Configuring done
```

### Build tunserver.elf.exe in Visual Studio:
- Open Visual Studio, "Open project", choose folder, where you generate MSVC project files
- Open `galaxy42.sln`
- Right mouse button (on "tunserver.elf" in Solution Explorer section) -> Build
- Copy dll library files to binary directory

	For solutrins configuration Debug
```
boost_filesystem-vc140-mt-gd-1_61.dll
boost_locale-vc140-mt-gd-1_61.dll
boost_system-vc140-mt-gd-1_61.dll
libsodium.dll
```
	For solutrins configuration Release
```
boost_filesystem-vc140-mt-1_61.dll
boost_locale-vc140-mt-1_61.dll
boost_system-vc140-mt-1_61.dll
libsodium.dll
```

__Possible errors:__

```Warning	D9002	ignoring unknown option -std=c++14```
- you should use compilator that support c++14 (probably you are using an older version of Visual Studio)

```Error	LNK1104	cannot open file 'pthread.lib'	tunserver.elf```
- simply remove pthread.lib from linking Additional Dependencies

```Severity	Code	Description	Project	File	Line	Suppression State```
``` Error	LNK2019	unresolved external symbol _CreateUnicastIpAddressEntry@4```
- add Iphlpapi.lib to Additional Dependencies that contain CreateUnicastIpAddressEntry function.

``` Error LNK1112 module machine type 'X86' conflicts with target machine type 'x64' ```
 or
 ``` Error LNK1112 module machine type 'x64' conflicts with target machine type 'X86'```
- make sure you link correct version of 32-bit or 64-bit (libsodium/libboost) libraries.


### Create TUN/TAP device
__download/install openvps drivers for windows tap device__
- [OpenVPN-Downloads](https://openvpn.net/index.php/open-source/downloads.html)

__set up device in windows devices manager (this step must be done as administrator)__
OpenVPN installer creates default one TAP windows adapter. If you need more adapters:

Windows 7

```my computer -> properties -> devices manager -> action -> add old device -> choose manually from list netwotk device -> "TAP-Windows Adapter V9"```

Windows 8 and newer

```my computer -> properties ->device manager -> action -> Add legacy hardware -> Install the hardware that I manually select from a list -> Network adapters -> Have disc -> locate tun driver (default path C:\Program Files\TAP_Windows\driver\OenVista.inf)```



### Possible runtime errors (tunserver.elf.exe):
***cannon open shared library***
```
	$ ./tunserver.elf.exe
	C:/cygwin/home/Tigu_WIN/msvc/galaxy42/Debug/tunserver.elf.exe: error while loading shared libraries: libsodium.dll: cannot open shared object file: No such file or directory
```
- manually copy libsodium.dll to dir that contain tunserver.elf.exe

- same error with boost libraries - manually copy missing boost libs to dir that contain tunserver.elf.exe

***Windows alert - abort()***

```Microsoft Visual C++ Runtime Library - debug Error! - abort()```
- reason - probably network TAP device is missing or your windows user don't have access to it (Do you created TAP device?).

***problem with TUN/TAP parser***

```tun data length < 54 -- in tunserver.elf.exe debug messages```
- solution: in network device properties for TAP-Windows Adapter V9 - uncheck all components instead of ipv6 protocol
