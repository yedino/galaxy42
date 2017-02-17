
On supported Linuxes, just run two commands:

```
./install.sh
./do
```

This should work on:
  * Debian 8 (amd64)
  * Devuan 8 (amd64)
  * Probably also other Linux Debian versions above 8
  * Ubuntu 15.10 (amd64)
  * Probably also other Linux Ubuntu versions above 15.10
  * Probably Fedora 25 (amd64)
  * Also on some related systems.

* * *

If the script install fails, you can try installing yourself the **dependencies** (libraries etc), and then try running `./do`

* * *

If that all fails, you can instead:

try just running normal `cmake . && make` **but** first you have to yourself do following:
  * install dependencies in system
  * git submodules update --init (or alike)
  * build local dependency libraries if needed (e.g. libsodiumpp) for ones that are not automatically built from our main cmake/make.

* * *

Static build

Static build of our program require static dependencies.

  Boost libraries from linux system repositories usually doesn’t provide by default static libraries. So to fill needed 
  cmake variables BOOST_INCLUDEDIR or BOOST_ROOT. We have to build boost manually from source with parameters: link=static and runtime-link=static
  boost build could be followed by $GALAXY_DIR/contrib/gitian-descriptors/libboost-build/build-boost-static

Build steps for native build (after manualy boost build):

	$ BUILD_STATIC=1 BUILD_SET_BOOST_ROOT="./path/to/boost/static-build" ./do --go

After successfull build we can check whether the result tunserver.elf binary is built correctly (does not require any shared library) by:
	$ ldd tunserver.elf
	not a dynamic executable

