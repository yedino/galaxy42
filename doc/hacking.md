
# Hacking

This page described how to "Hack" this project - how to develop it, how to change it, how to build it.

Intended for:

+ developers of this project
+ packagers, maintainers of this project
+ power users building own versions or modding this project

# Building

The build process should be described in the main README.md of this project.

# Using

The use of this project/programs as end-user, is described in other places, see the main README.md of this project.

# Developing

To develop the program, we recommend mainly Debian Stable as the main environment at this time.

Contact us for any hints (be patient waiting for reply, but fell free to ask in few places or few times to catch us if
we're bussy), see contact information in the main README.md of this project.

# Naming and Glossary


* Hash-IP - the IP that someone can seldomly own without requiring permission from any authority, because it is derived from public key with some form of Hash.
It will be in form of IPv6 (because other option of IP - IPv4 - is not suitable as it's too short).

* Hash-IP is a global IP - this is an IP, where you are the only owner of it, and no one else have such IP even globally.

* Mapped-IP is an IP that is owned by you but that is guaranteed only in given Hash-VPN. It is usually created by mapping e.g. from Hash-IP to some private IPv4 (RFC1918).

* Hash-VPN is a VPN network where all participants communicate through Hash-IP.

* IP-Key - the cryptographical key that you use to have ownership of your Hash-IP.

# Special topics

## EXTLEVEL
{{extlevel}}
EXTLEVEL is a setting of the project, that decides do we want a stable version, or preview or experimental version.

### For
This setting is for developers and possibly for project builder (packager-men).

### Using
When building the project with the ./do script, you can set EXTLEVEL to some value, e.g. `EXTLEVEL=10 ./do`,
see the script ,
see the script `./do --help` for details and list of available options.

### Developing: changing what given option means

CMakeLists.txt - should contain the main list of possible EXTLEVEL-s, and also information which feature (e.g. crypto SIDH)
is on which EXTLEVEL, look for `{{match_extlevel_*}}`.

To move e.g. SIDH to other EXTLEVEL, search all lines with e.g. `{{match_extlevel_sidh}}`.

Usually it is comfortable, that is the CMakeList sets a concrete flag like `ENABLE_CRYPTO_SIDH` (that is passed to C++),
but remember also about do-scripts (e.g. `./do`, `build-extra-libs.sh`).

### Developing: writting a function that has some EXTLEVEL

#### If a library, then

+ build system - choose if the lib will be builded by bash script or by your CMake, and either:
 + build-extra-libs.sh - add a proper `if` for it, checking the EXTLEVEL
 + CMakeLists - add on top a new e.g. `{{match_extlevel_newthing}}`
  + Add a block that sets option like `ENABLE_NEWTHING`, and elsewhere do if on that option and add link-flags,
  and `add_definitions` to export it into C but as `ENABLE_NEWTHING_CMAKE`
+ in C++ in project.hpp define "rename" the `ENABLE_NEWTHING_CMAKE` to `ENABLE_NEWTHING` (so we can doxygen-comment it there)
+ in C++ use the code inside `#if ENABLE_NEWTHING ... #endif`
 + if the code tries to e.g. open data created previously with enabled given feature, but now this feature is disabled, then
 signal error by throwing special exception `invalid_argument_in_version`.
 + but if the error can be ignored (e.g. negotiating which crypto systems to use) then just do not use it
 (and log it at e.g. `_dbg2`).

#### If other setting, then:

+ in C++ code use it simply by checking the EXTLEVEL like `#if EXTLEVEL_IS_NORMAL` (see project.hpp for alternatives)
+ or create an `ENABLE_NEWTHING` as described for library and use it like that


