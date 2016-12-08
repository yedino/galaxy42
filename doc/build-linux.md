
To build the program on Linux on supported systems, that is:
  * Debian 8 (amd64)
  * Devuan 8 (amd64)
  * Probably also other Linux Debian versions above 8
  * Ubuntu 15.10 (amd64)
  * Probably also other Linux Ubuntu versions above 15.10
  * Probably Fedora 25 (amd64)

run script:

`./install.sh`

and then run script: `./do`

If the script install fails, you can try installing yourself the **dependencies** (libraries etc), and then try running `./do`

Other then that, you can try just running normal `cmake . && make` but then you have to yourself
  * install dependencies in system
  * git submodules update --init (or alike)
  * build local dependency libraries if needed (e.g. libsodiumpp) for ones that are not automatically built from our main cmake/make.

