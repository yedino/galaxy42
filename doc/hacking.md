
# Hacking

This page described how to "Hack" this project - how to develop it, how to change it, how to build it - topics for developers.

Intended for:

+ developers of this project
+ packagers, maintainers of this project
+ power users building own versions or modding this project

# Summary for every developer!

Know the Dictionary (see below) and always use that (in code, doc, materials, bugtrackers).

Use ./menu
Possibly use [../doc/cmdline/](../doc/cmdline/) file to just use `make run`.

```cpp
// TODO  ....  TODO@author
thing_to_be_fixed_before_release ; // TODO-release

is_ascii_normal // Are all chars "normal", that is of value from 32 to 126 (inclusive), so printable, except for 127 DEL char

assert() / _check_abort() / _check()
1. abort on error (only guaranteed in debug mode) - assert() // from compiler
2. abort on error (always guaranteed) - _check_abort() // our lib
3. throw on error - _check() // our lib

Function: if throw - then std::exception (or child class).
Member functions: assume are not thread safe for concurent writes to same object, unless:
// [thread_safe] - thread safe functions.
auto ptr = make_unique<foo>(); .... UsePtr(ptr).method();

Throw:
_throw_error_runtime("TTL too big");
_throw_error( std::invalid_argument("TTL too big") );
_throw_error_runtime( join_string_sep("Invalid IP format (char ':')", ip_string) );

try {
	_check( ptr != nullptr ); // like assert
	_check_user( size < 100 ); // user given bad input
	_check_sys( file.exists() ); // system doesn't work
	_check_extern( connection_format == 2 ); // remote input (e.g. peer) given wrong data

	int i=2;
	assert( i+i == 4); // obvious, no need to check it
	auto size2 = size*size;
	_check_abort( size2 >= size ); // almost obvious, especially since we already check size<100,
	// it would happen if someone would set "size" type to be e.g. unsigned char

	// same, but we expect this errors as they are common and we do not want to spam log with ERROR
	_try_user( size < 100 ); // user given bad input - common case
	_try_sys( file.exists() ); // system doesn't work - common case
	_try_extern( connection_format == 2 ); // remote input (e.g. peer) given wrong data - common case
}

Catch it using:
  catch(std::runtime_error &ex) // catch all errors, including check soft and hard errors
-or-
// catch soft (expected) error, but hard errors propagate
  catch(err_check_soft &soft) { string info = soft.what_soft(); }
// for more see chapter Check-asserts
```

# Building

The build process should be described in the main README.md of this project.

See what dependencies you need to monitor for upgrades! See: [SECURITY.txt].

* For CI (continuous integration) we use
  * Jenkins (see below in Using-Jenkins)
  * Travis (see below in Using-Travis)

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

## Developing and code details

# Developer handbook

## Check-asserts

Read also the Summary chapter first. More details are in file **utils/check.hpp** .

You can also catch:
```
catch(err_check_user &ex) { string info = ex.what(); } // catch error (soft of hard) caused by user input
catch(err_check_sys &ex) { string info = ex.what(); }  // catch error (soft of hard) caused by system
catch(err_check_extern &ex) { string info = ex.what(); } // catch error (soft of hard) caused by external
```

### Startup of TUN/TAP card

Startup and error reporting is as follows:

```
1) low-level code, in pure C, reports error via syserr.h struct t_syserr
NetPlatform_addAddress(..) and NetPlatform_setMTU(..)
depends/cjdns-code/NetPlatform_darwin.c
depends/cjdns-code/NetPlatform_linux.c

2) medium code, in C++, reports errors via exceptions:
	Wrap_NetPlatform_addAddress(..) - throws
	Wrap_NetPlatform_setMTU(..) - throws
this is called by:
	c_tun_device*::set_ipv6_address(..)
	c_tun_device*::set_mtu(..)
and this throws proper tuntap_error*

3) tunserver.c will catch then tuntap_error* and inform user
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

# Build infrastructure and tools

## Continuous Integration (CI) tools

We use both Jenkins and Travis.

Travis is easier to set up (integrated with github.com), but can be slow, you need 3rd party machines, and is less secure in a way.
We will run simple tests on it.

Jenkins requires a set up (installation of master and slave nodes) but is more open, you can run all on own (+volounteer's) machines,
is therefore more secure, and can be much faster if you can provide strong boxes.
We will run more advanced tests on in that require more resources or not supported on Travis platforms.

### Travis

To use Travis just:

* fork our project
* create account on Travis http://travis-ci.org/
* connect travis-ci.org to your e.g. github.com (or other git account if they allow that)
* modify file .travis.yml that is on top of this project's source code - set email addresses / notifications and possibly change tests

that should be enough: now when you push to your git account associtated with your travis-ci.org account,
then it should run the build and notfiy you about results.

### Jenkins

To use Jenkins:

* install Jenkins, on your master computer. Connect to it via webUI and administrate.
* configure Jenkins tests. We will write and export the recommended tests set, in future.
* on Slave nodes, probably install Jenkins and give SSH access to it from the Master
* on Master node, add the Slave nodes credentials
* set Jenkins to observe your git account for updates and to run tests

Our Jenkins test suite defined by one pipeline can be checked in details in the Jenkinsfile located in the root directory of the project.
At the moment test suite includes:
	- native build on gcc debian8 linux and mingw/cygwin 32-bit windows
	- unit tests passing
	- integration tests passing
	- deterministic build linux target
	- deterministic build osx target
	- deterministic build windows installer

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

## Dictionary - common naming:

Dictionary, dict:

* "privkey" - Private Key - is the private (secret) key in [Public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography)
* "pubkey" - Public Key - is the public key in [Public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography)
* "macosx" - Mac OS X / Macintosh operating system
* "tuntap" - the TUN/TAP virtual network technology/drivers/cards
* "p2p" - "peer2peer" - [e.g. connection] to a direct peer
* "p2p tunnel" - crypto tunnel for p2p
* "e2e" - "end2end" - [e.g. connection] as end-to-end, e.g. to transport user data form IPv6 o IPv6
* "e2e tunnel" - crypto tunnel for e2e
* "cable" - is a logical cable, an underlying "transport" mechanism. e.g. UDP4, or ETHGAL, or Email - to deliver our data p2p
* "ETHGAL" - would be our raw IP Ethernet transport to deliver our data to MAC address in Ethernet network

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

# Technical docs

## RPC protocol

```

RPC/Json for GUI (now being developed)

There are two Sides of RPC use.

Sides send eachother messages. Each message has JSON text in it (utf8).

Messages are converted into armoredmessages by adding few (1+4 = 5) bytes in front.
Armoredmessages are merged together into a bytestream.
This bytestream is cut into chunks and sent using some transport.
A transport could be TCP connection to localhost e.g. implemented with boost::asio.

Ech armoredmessage has following format: (byte means octet with unsigned number 0..255)

1 byte 0xFF
var int of 1,2,4 or 8 bytes representing N
N bytes - the message

Transport must deliver all chunks without loss and in-order.

1 chunk can have 0, 1, or more armmsg or their parts (begining of armmsg, middle part of it, one of middle parts of it,
the end of it, or entire).

Example:
message: {msg:"ok"}
armmsg:  0xFF 0x0A {msg:"ok"}
message: {foo:123}
message: 0xFF 0x09 {foo:123}
stream:  0xFF 0x0A {msg:"ok"} 0xFF 0x09 {foo:123}
any possible chunking can appear.
it can even cut away the "header" of armmsg

TODO verify this with example GUI code.

```
