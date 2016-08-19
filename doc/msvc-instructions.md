# GALAXY42 compilation on msvc (Visual Studio 2015):

### Process instructions for:  

- version:  
	```commit ca9aefcdc45f2b4dc76fd83e5de9a22636459ed9```
	- It should be similar or easier for newer versions

### Used tools:
- Cygwin for 32-bit versions of Windows (for git)  
    - could be other tool that supports git
- Visual Studio Community 2015
- Cmake-GUI 3.6.0

### Cygwin:
```sh
$ git clone https://github.com/yedino/galaxy42
$ cd galaxy42
$ git submodule update --init
$ git log -1
commit ca9aefcdc45f2b4dc76fd83e5de9a22636459ed9
```
### Cmake-GUI 
Creating project for Visual Studo (.sln and .vcxproj's) using Cmake-GUI
- settings:  
```
Where is the source: 		path/to/cloned/galaxy42  
Where to build binaries:	any/output/path
```
```
click configure
click generate
```
- cmake output :
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
### Project configuration in Visual Studio 2015:  
- remove unrecognized compilation flags:  
in main tunserver.elf and all coresponding libs  
	- -Wextra  
	- -Wno-unused-command-line-argument   
```right button -> properties -> C/C++ -> All Options -> Additional Options -> [remove flags]```
	- same step in json_lib_static and sodiumpp 

- provide dependencies (libboost and libsodium):  
	- pre-compiled binaries could be found on official pages.  Download and install/unpack libraries.
	I used:  
		- [libsodium releases](https://download.libsodium.org/libsodium/releases/)  
		- [boost for msvc 1.6.1](https://sourceforge.net/projects/boost/files/boost/1.61.0/)

- add path for dependencies in visual studio project properties:  
```right button -> properties -> C/C++ -> All Options -> Additional Include Directorie```  
    - add paths to libboost and libsodium (example):
```
		    C:\local\libsodium-1.0.11-msvc\include;  
		    C:\local\boost_1_61_0;
```
- set character set to unicode:  
```Configuration Properties -> General - > Character Set -> Use Unicode Characer Set```

- link sodium.lib in visual studio:  
```right button -> properties -> Linker -> All Options -> Additional Library Directories```  
        - make sure that you have typed libsodium.lib" in Additional Dependencies  
```right button -> properties -> Linker -> All Options -> Additional Dependencies```
	- Additional Library Directory example:
```
		    C:\local\libsodium-1.0.11-msvc\Win32\Release\v140\dynamic
```  

- link boost libraries in visual studio:  
```right button -> properties -> Linker -> All Options -> Additional Library Directories```
	- Additional Library Directory example:		
```
			C:\local\boost_1_61_0\lib32-msvc-14.0
```
- it could be necessary to changes boost libs names in  Additional Dependencies:  
    - in my case:
```
	        boost_system.lib -> boost_system-vc140-mt-gd-1_61.lib
			boost_filesystem.lib -> boost_filesystem-vc140-mt-gd-1_61.lib
			boost_program_options.lib -> boost_program_options-vc140-mt-gd-1_61.lib
```

		problem has occurred with boost_program_options you could remove boost_program_options-vc140-mt-gd-1_61.lib from  Additional Dependencies


### Build tunserver.elf.exe in Visual Studio:
``` right button (on "tunserver.elf" in Solution Explorer section) -> Build```  
- possible errors:  
    ```Warning	D9002	ignoring unknown option -std=c++14```
    - you should use never compilator that support c++14 (probably you are using an older version of Visual Studio)

    ```Error	LNK1104	cannot open file 'pthread.lib'	tunserver.elf```
    - simply remove pthread.lib from linking Additional Dependencies  
    
    ```Severity	Code	Description	Project	File	Line	Suppression State```  
    ``` Error	LNK2019	unresolved external symbol _CreateUnicastIpAddressEntry@4```  
	- add Iphlpapi.lib to Additional Dependencies that contain CreateUnicastIpAddressEntry function.


**If soudiumpp lib is deprecated it could be built standalone. In my case:**  
- Cygwin:
```sh
	$ git clone https://github.com/robertoleksy/sodiumpp.git
	$ git checkout origin/tigusoft   ('in my case for commit:' cf425da74f86364a3672d7f4894752f463a153b0)
```
- Cmake-GUI
	- add bool entry ```SODIUMPP_STATIC``` to create static lib sodiumpp.lib instead of dynamic sodiumpp.dll
    - rest similar to [cmake for galaxy42](#cmake-gui)  
- Visual Studio:
	- open and build sodiumpp project in visual studio (steps similar to galaxy42)
	- this step also need to manual add include and link directories

### Create TAP device
```my computer -> properties -> divices manager -> action -> add old device -> choose manually from list netwotk device -> "TAP-Windows Adapter V9"```
		
		
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
- reason - probably network TAP device is missing (Do you created TAP device?).

***problem with TUN/TAP parser***  

```tun data length < 54 -- in tunserver.elf.exe debug messages```
- solution - uncheck all instead of ipv6 protocol in network device properties 
