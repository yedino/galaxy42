
# Hacking

This page described how to "Hack" this project - how to develop it, how to change it, how to build it.

Intended for:

+ developers of this project
+ packagers, maintainers of this project
+ power users building own versions or modding this project

# Building

The build process should be described in the main README.md of this project.

See what dependencies you need to monitor for upgrades! See: [SECURITY.txt].

# Using

The use of this project/programs as end-user, is described in other places, see the main README.md of this project.

# Developing

To develop the program, we recommend mainly Debian Stable as the main environment at this time.

IF YOU ADD ANY DEPENDENCY THEN write it in dependencies list in [SECURITY.txt].

Contact us for any hints (be patient waiting for reply, but fell free to ask in few places or few times to catch us if
we're bussy), see contact information in the main README.md of this project.

## Developing and testing Install-code

Install-code is the installer `./install.sh` that you run from place where you have (ready - e.g. unpacked, or better: downloaded from git with `git clone`)
source code of this project. (Remember to check git-tag signature and/or write down and compare with others git revision hash - to confirm git downloaded legitimate version of code).

To test installer for other languages and conditions you can use combinations of settings. Possible conditions:
- force to use dialog, or whiptail. Other test: run on system with just one or both installed (dialog should be preffered)
- force a language.
- run as root
- run as non-root, with sudo privilages
- run as non-root, without sudo privilages (install will be not possible)

```
# as root:
FORCE_DIALOG=whiptail LANGUAGE=en ./install.sh
FORCE_DIALOG=dialog LANGUAGE=pl ./install.sh

# as non-root user, but allowed to "sudo" into needed commands especially the package manager:
FORCE_DIALOG=dialog LANGUAGE=pl ./install.sh --sudo
```

## Developing translations

### Writting the code in translatable way

#### Bash

See ./install.sh for example how to (gettext)

### Preparing translation files

Update existig file:

Use script:

```
contrib/tools/lang-updater  -h

# for example:
contrib/tools/lang-updater  -t -u
```

Older method was to use commands like:

```
xgettext -j  -o src-misc/locale/galaxy42_installer/messages.pot  install.sh
msgmerge --update src-misc/locale/galaxy42_installer/pl.po   src-misc/locale/galaxy42_installer/messages.pot
msgfmt --check --endianness=little src-misc/locale/galaxy42_installer/pl.po -o ./share/locale/pl/LC_MESSAGES/galaxy42_installer.mo
```

(though this command probably do not e.g. remove obsolete strings etc)

# Naming and Glossary

## Our naming (in Galaxy42, Antinet, Yedino)

* Hash-Node (or just "Node") - is some sort of computer system that has a Hash-IP, and usually is connected with it to some network.
For Galaxy42, a Node will be any computer running the Galaxy42 client program.
One computer can run several nodes at once, using e.g. network name-spaces. And that all can be run at once few times on one physical computer with help of e.g. virtual machines.
But usually for simple use, one computer runs one Galaxy42 client program and is therefor an Hash-Node.

* Hash-IP or "Hash-IP address" - is a virtual IP address, that is based upon a **cryptographic hash** and is derived from some **public key**.
Use of given Hash-IP as owner of this address (treceiving data addressed to this IP, or sending data from this IP) demands access to the associated **IP-Key-Secret**.
Hash-IP is a **globally-unique IP** address - where can choose to be the only owner of it, and no one else have such IP even globally (though given address can potentially
be multi-homed or shared across many users, but this is their decission).
(This assumes that all works as expected and the used Hash function is strong enough to avoid any collisions).

* Mapped-IP - is an IP that is owned by you but that is guaranteed only in given Hash-VPN.
It is usually created by mapping e.g. from Hash-IP to some private IPv4 (RFC1918) by admin of such Hash-VPN.

* Hash-Net - is some network of Nodes connecting to each other through each-other Hash-IP. It usually forms either Hash-VPN or Hash-Mesh.
 * Hash-Mesh - is kind of Hash-Net, where every Node can join. Usually it is desirable that such mesh can reach every other existing Hash-Mesh.
 * Hash-VPN - is a private version of Hash-Net (like a VPN but in Hash-Net). It is run under some sort of common administrative entity
 (e.g. an admin of VPN, who assigns Mapped-IP to avoid collisions). Usually it is desirable that such mesh will NOT be used by
 any other, unauthorized Nodes.
 * Hash-VPN-1to1, or (preferred) "1to1-VPN" - is Hash-VPN in which there are only (up to) 2 Nodes connecting to each-other.

* IP-Key-Secret - is the cryptographical **private key** that you use to have ownership of given Hash-IP.

* IP-Key-Public - is the cryptographical **public key** that you use to have ownership of given Hash-IP.

For more of technical names, see also source code file: [crypto.hpp](../src/crypto/crypto.hpp)

## Common naming

* Private Key - is the private (secret) key in [Public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography)

* Public Key - is the public key in [Public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography)

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


