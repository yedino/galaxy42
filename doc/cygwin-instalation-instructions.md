### Instruction for Cygwin download/instalation.



#### 1. Download Cygwin installer:
###### You can download if from here:
- 32-bit installation: https://www.cygwin.com/setup-x86.exe
- 64-bit installation: https://www.cygwin.com/setup-x86_64.exe

#### 2. Install Cygwin:
1. Run downloaded file
2. -> Next
3. "Install from internet" -> Next
4. Your settings -> Next (recomended basic)
5. Your settings -> Next (recomended basic)
6. "Direct Connection" -> Next
7. -> Next
8. Choose packeges to download
9. -> Next
10. -> Finish

#### 4. Choosing packeges:

Example:
- search git
- click on Devel
- find line witch "git: Distributed version control system"
- click several time on Skip/Keep in this line and choose latest version

##### 5. Needed packeges:
###### Convention:
  search:     click on:       [line to find]
- git:        Devel:          [git: Distributed version control system]
- boost:      Devel + libs:   all
- sodium:     Devel + libs:   all
- libtool:    Devel:          [libtool: Generic library support script]
- cmake:      Devel:          [cmake: Cross-platform makefile generation system]
- wget:       Web:            all
- make:       Devel:          (make: GUI version of the 'make' utillity)
- gcc:        Devel:          mingw64-i686-gcc-g++: GCC for Win32 (i686-w64-mingw32) toolchain (C++)
- gcc:        Devel:          mingw64-x86_64-gcc-g++: GCC for Win64 toolchain (C++)
