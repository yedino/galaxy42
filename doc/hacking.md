@page Hacking
# Hacking

This page described how to "Hack" this project - how to develop it, how to change it, how to build it - topics for developers.

Intended for:

+ better bug-reports, debugging, testers
+ developers of this project
+ packagers, maintainers of this project
+ power users building own versions or modding this project

# Basic environment setup

Run ./install.sh and set up the tools.

As developer, you might add to ~/.bashrc lines to choose clang++ as compiler, or to choose given version of it,
and line to wrap in ccache the compiler for fast recompile.
E.g. add to ~/.bashrc :
```
export CC="clang-4.0" ; export CXX="clang++-4.0" ; export CPP="clang++-4.0 -E"
export CC="ccache $CC" ; export CXX="ccache $CXX" ; export CPP="ccache $CPP"
```

# Debugging

Run program (e.g. ./tunserver.elf) with option --d for debug.
Also applies to many run scripts like: ./run-safe-thread-ub --d , ./run-safe-mem --d , ./rundevel.sh --d.

## Debug tools

Possible goals for tools:
+ thread correctness: clang TSAN tool. (plus source-code uses clang Thread-Safety-Analysis).
+ UB other (including overflows): clang UBSAN tool.
+ memory correctness: valgrind (memcheck) tool.

Tests you should run threfore (separate tests):

1) Test for Thread and UB:
a) build version with TSAN and UBSAN options in ccmake (requires clang compiler to be set first!).
b) and run ./run-safe-thread-ub - look for errors.

This will run as-root (at least) on some systems - instead of running as-user the normal version (tunserver.elf that has setcap)
...due to clang bug(?): abort-on-error seemingly not working correctly sometimes (e.g. debian 8)

Do check/demonstrate how this tool is working, run it with options (to trigger TSAN/UBSAN errors - then this tests should abort
and show error messages for this example bugs in code):
`
./run-safe-thread-ub --special-tsan1=ON
./run-safe-thread-ub --special-unsan1=ON
`

2) To run tests for Memory, using valgrind:
a) build normal version (without ccmake options TSAN nor ASAN; UBSAN is allowed if you want)
and run  ./run-safe-mem
This will run as-root (at least) on some systems - instead of running as-user the normal version (tunserver.elf that has setcap)
...due to valgrind bug(?): refusing to run capability (setcap) binaries.

Example, this option --special... will trigger a demonstration error and abort:
`
./run-safe-mem --newloop --special-memcheck2=ON
`

### FAQ for the Debug Tools

Q: Program can not run inside valgrind, tool says `valgrind_memory_is_possible`

Q: Running in valgrind (memcheck tool) and it shows warnings `Warning: set address range perms: large range ... (noaccess)`
and then dies.

A: Program using TSAN can not be run in valgrind, use ccmake to disable TSAN flags and rebuild - when you want to run in valgrind.

# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

# Summary for every developer!

Know the Dictionary (see below) and always use that (in code, doc, materials, bugtrackers).
Know the Editor (below).

Know the Code Style - see example in .cpp file: [[.//style_example.cpp]] (src/style_example.cpp).

Use ./menu
Possibly use [../doc/cmdline/](../doc/cmdline/) file to just use `make run`.

Try to minimize technical debt by fixing things in [debt.txt](debt.txt).
If you must move forward without writting things the good way for now, add information to that debt.txt.

Cheat sheet (this is compact/mnemonics, more elaborated version is later on; ask us if any questions)

```cpp
_dbg4() _dbg3 _dbg2 _dbg1 _info _note _clue _fact _warn _erro _mark

auto x = xint{ 5 };
xint y = 5;

Handling exceptional situations:
	_check_abort()                   --> will abort (guaranteed)
	abort()                          --> can abort, but not always, e.g. only in debug mode
	throw critical_exception("..."); --> catch(const some_special_exception&) / catch(const critical_exception&)
	_check_input()     (this catch,) --> catch(const err_check_input&) also _user _sys _extern
	_check_input()     (or this one) --> catch(const err_check_soft&) - all soft errors
	_check_sys()    - same
	_check_user()   - same
	_check_extern() - same
	_check()                         --> catch(const std::runtime_error&) catch with other errors, or only at top/main
	_throw_error_runtime(...)        --> catch(const std::runtime_error&)
	_throw_error_runtime("TTL too big");
	_throw_error( std::invalid_argument("TTL too big") );
	_throw_error_runtime( join_string_sep("Invalid IP format (char ':')", ip_string) );
	_try()                           --> catch(const err_check_soft&)
Note: the abort() is provided by default by compiler.

STR(...) to_string(...) "..."s
to_debug(...);
is_ascii_normal(str); reasonable_size(str); reasonable_size(vec);
UsePtr(p).func();

/**
 * @codestyle
 * @thread
 * @owner rfree
 */
// TODO@author

catch(const ex_type & ex) {}

UNUSED(x); DEAD_RETURN();
```

```cpp
_dbg4(X) // unorganized "removed" debug
_dbg3(X) _dbg2(X) _dbg1(X) // debug "Load key=0x1234"
_info(X) _note(X) _clue(X) // information, more important, bigger group
_fact(X) _goal(X) // very improtant goals shown to user
_warn(X) _erro(X) // warnings/errors
_mark(X) // hot topics (usually for testing)

// TODO@author

is_ascii_normal(str) // Are all chars "normal", that is of value from 32 to 126 (inclusive), so printable, except for 127 DEL char

Function: if throw - then std::exception (or child class), or critical_exception.
Member functions: assume are not thread safe for concurent writes to same object, unless:
// [thread_safe] - thread safe functions.
auto ptr = make_unique<foo>(); .... UsePtr(ptr).method();

try {
	_check( ptr != nullptr ); // like assert
	_check_user( size < 100 ); // user given bad input
	_check_input( size < 100 ); // bad input
	_check_sys( file.exists() ); // system doesn't work
	_check_extern( connection_format == 2 ); // remote input (e.g. peer) given wrong data

	int i=2;
	assert( i+i == 4); // obvious, no need to check it
	auto size2 = size*size;
	_check_abort( size2 >= size ); // almost obvious, especially since we already check size<100,
	// it would happen if someone would set "size" type to be e.g. unsigned char

	// same, but we expect this errors as they are common and we do not want to spam log with ERROR
	_try_user( size < 100 ); // user given bad input - common case
	_try_input( size < 100 ); // bad input - common case
	_try_sys( file.exists() ); // system doesn't work - common case
	_try_extern( connection_format == 2 ); // remote input (e.g. peer) given wrong data - common case
}

reasonable_size( vec ); // <--- test potentially big objects on input, function start

Catch it using:
  catch(const std::runtime_error &ex) // catch all errors, including check soft and hard errors
-or-
// catch soft (expected) error, but hard errors propagate
  catch(const err_check_soft &soft) { string info = soft.what_soft(); }
// for more see chapter Check-asserts

enum class t_temper { cold=15, hot=80 }; // for (de)serializable Enums. See "Enum" below.
inline bool enum_is_valid_value(t_temper value) {
	switch (value) {
		case t_temper::cold :
		case t_temper::hot :
		return true;
	}
	return false;
}

t_temper water_temp = int_to_enum<t_temper>( 80 ); // asserted

```

# Doxygen tags attributes

## Attribute @thread

Popular kinds of thread safety used in this project:

@thread `thread_safe` - applies to function: for member function it means that it is safe to call object of this class
concurrently from different threads without any additional synchronization by caller;
for free function it means that it is safe to call this free function
concurrently from different threads without any additional synchronization by caller;

@thread `thread_safe_public_myself` - applies to a class - means "Thread Safe on Public, if I'm just using Myself":
all public function are `thread_safe` unless noted otherwise,
except for all constructors (including move and copy), destructor,
and except for functions and operators that take same-class (or parent/child class) - like especially copy operator=,
moving operator=, comparison operator< operator> operator== and such.

## Attribute owner

Owner of the file - e.g. nickname.

# Building

The build process should be described in the main README.md of this project.

See what dependencies you need to monitor for upgrades! See: [SECURITY.txt].

* For CI (continuous integration) we use
  * Jenkins (see below in Using-Jenkins)
  * Travis (see below in Using-Travis)

# Using

The use of this project/programs as end-user, is described in other places, see the main README.md of this project.

# Developing

To develop the program, we recommend mainly Debian Stable (Debian 9 Stretch, Amd64) as the main environment at this time;
Ubuntu and Mint also are used by developers so we can recommend them (at least in some versions),

IF YOU ADD ANY DEPENDENCY (library) THEN write it in dependencies list in [SECURITY.txt].

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

## Editor

### Editing source code (and in general text files)

Identation is done with tab-characters.
Files should be in UTF-8, using Unix line-end markers (exceptions allowed for files edited on windows, temporarly).

We assume/recommend to display tabs as 2-character-wide.
We recommend keeping lines-length below 100-120, max 130 collumns (with following tab size).

Following comment-line horizontal ruler ornaments are used (given here below as `.vimrc` macros to be pasted);
Optionally, following marking of leave-block instructions (to be used especially when block is exited suddently instead at normal end of it)

Recommended vimrc settings to use if that is your editor:

```vim
:set noexpandtab
:set copyindent
:set smarttab
:set smartindent
:set softtabstop=0
:set shiftwidth=2
:set tabstop=2

iabbrev !!# // ###########################################################################################################
iabbrev !!= // ===========================================================================================================
iabbrev !!- // -------------------------------------------------------------------
iabbrev !!r return ; // <=== return
iabbrev !!b break ; // <=== break
```

## Developing and code details

# Developer handbook

## Check-asserts

Read also the Summary chapter first. More details are in file **utils/check.hpp** .

You can also catch:
```
catch(const err_check_user &ex) { string info = ex.what(); } // catch error (soft of hard) caused by user input
catch(const err_check_sys &ex) { string info = ex.what(); }  // catch error (soft of hard) caused by system
catch(const err_check_extern &ex) { string info = ex.what(); } // catch error (soft of hard) caused by external
```

## Enum

When using enums, try to use enum-class.
If given enum needs to be (de)serializable or otherwise convertible from-integer, then provide override of function:
`inline bool enum_is_valid_value(t_your_enum_type value);`
that will return true if given enum has allowed value in it; else false. See unit tests (`stdplus_misc.cpp`) for example.

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

## Network protocol

`[[maindoc]]` - main documentation for network protocol

tun from tuntap - are the IP data
tap from tuntap - are the ethernet data, including usually IP data after ethernet header

tuntap-data - the data as received by some tuntap driver
tuntap-header - the header added by some tuntap driver, before the IP data, e.g. 4 bytes
tuntap-added - all data added before the TUN

ipv6-noaddr: are the headers of IPv6 and payload of IPv6, but with source and dest addresses removed
ipv6-merit: currently = ipv6-noaddr; Data that are are part of IPv6 and are not "trivial" to know from higher layers.

weld: array of ipv6-merit, that are all from same src, and to same dst
e.g. ipv6-merit are joined into a weld by the original sender, delivered e2e, and then separated by final recipient.
they are e2e encrypted and authenticated by src to dst.

```

=======================================================================================================================

Overview

Node can do 3 things: send locally-originating own data, receive data send to us, or route data between other nodes.
Each action can have Motivation effects like sending/receiving tokens of gratitude.

1) Sending data that we send locally:
from TUNTAP (+packages) ---> (crypt-e2e, crypt-p2p) (+our or other bags) ---> cable send, e.g. UDP transport
2) Receiving data that is sent to our host as end destination:
cable receive, e.g. UDP transport ---> (decrypt-p2p) (-bags not to us) ---> (-split bags) ---> (decrypt-e2e) ---> to TUNTAP
3) Receiving data that is not to us:
cable receive, e.g. UDP transport ---> (decrypt-p2p) ---> (encrypt-p2p other peer) ---> to cable e.g. UDP

        (emit)
    tuntap read          cable read UDP
      v    v                   |
      .    |                   |
 more ....>|                   |
 tuntap    |                   |
           v                   v
       e2e-crypt       p2p-auth(verify)
       e2e-auth(out)  [opt. p2p-encrypt]
           |                   |
           |                   |
           |<----- no ---- < to us ? >
           |   (routing)       |
           |                  yes
           |                   |
           v                   v
      p2p-auth(send)       e2e-decrypt
           |              e2e-auth(out)
           |                   |
           v                   v
         cable            tuntap write
                             (take)

=======================================================================================================================

Overview of dataflow for Sending local data:

TODO: We assume that there is available a function AuthenticateEncrypt that reads from separate (fragmented) buffers,
and outputs results to a new continuous memory.

Tuntap read gives us Ip Packets.

IP Packets  - Packet, as 2 buffers (IP headers - src,dst, IP payload) +src,dst
Merit       - Packet, with removed src/dst headers (as Src/Dst can be provided by higher level)
EmitInput   - Is concatenation of 1 or more merits, done to prepare less fragmented memory for Weld.
WeldBagStep - Produces Bags that are of mostly continuous memory and are e2e encrypted+authenticated,
              processed as: Weld ---> WeldPost ---> Bag, as follows:
Weld        - Weld is collection of IP packets, with same Dst, in form of array of Emitbuf, with fragmented memory.
              Logic: (src,dst)<===ipv,flow,.ip payload.===><===ipv,flow,.ip payload..===>
              Memory: (src,dst) EmitIn1{<=ipv,flow,payload=>} emit2{<=ipv,flow,payload=><=ipv,flow,payload=>
Bag         - Splited parts of some Weld. Small enough to usully be optimal for expected transport.
BagCrypto   - Bag that has e2e crypto applied, and includes nonce.
              nonce_e2e = bag_id (+) weld_id (+) salt
              bag_id is counter of bag, in context of weld_id
              weld_id is counter of weld, in context of my src (sender) to given finall dst
Cart        - Cart is collection of bags plus path-info, that is sent from us to given peer; It is p2p authorized.
              {
              p2p_authtag, p2p_nonce
              path-{B,C}-3,thx5005, passhash,   bag: <== weld-id, bag-id / bag-count , bag-data part of weld data ==>
              path-{B,C}-3,thx5006, passhash,   bag: <== weld-id, bag-id / bag-count , bag-data part of weld data ==>
              path-{B,D}-9,thx3111, passhash,   bag: <== weld-id, bag-id / bag-count , bag-data part of weld data ==>
				      }
				  		p2p_nonce = p2p_counter_in_path_hop_context (+) salt in hop_context
						  hop_context is: my-peer (implied), plus other peer (known from other layer), e.g. "A,B"
Fragment   -  Fragment is a part of cart that fits into given CableMTU
              cart#5811{B,C}, fragment nr 3/7, [.....cart-part....]

Sizes of data:
MTU on tuntap: 1304..65535 (usually 65535)
MTU on cable: 68..65535 (usually 1280..9000) (maybe bigger for jumbograms on cable)
MTU on path: same as MTU on cable
Packet: 40..65535 (possibly larger for IPv6 Jumbograms https://en.wikipedia.org/wiki/Jumbogram)
Merit: 8..65503 (as 65535-32)
maxBigPerWeld - 10 (1..1024) - estimated amount of big (MTU size) packets in Weld
maxPckPerWeld - 65530 (last few values can be reserved for other use)
EmitInput: ~60 .. maxBigPerWeld×MTU
Weld: ~60 .. maxBigPerWeld×MTU
WeldHeader: 0 ?
WeldPost: ~60 .. Weld.Max + Weld.Header.Max
BagHeader: ??? < 16 ??? PathAgreementID is needed
Bag: 1..65535 (usually 1..PathMTU) (maybe bigger for jumbograms on cable)
Cart: 1..65535 (usually 1..CableMTU) (maybe bigger for jumbograms on cable) - can cause fragmentation
maxFragPerCart: 65535 (encoded as varint)
ShredHeader: 2..2×3  + (cart id - 32 bit, rotating counter)
Shred: 1+header .. 65535

Max nonce for authencrypt: 24 byte (crypto_stream_xsalsa20.h: #define crypto_stream_xsalsa20_NONCEBYTES 24U)

FAQ:
Does Bag need Dst (finall destination) info? No, because when Bag is handed to you in p2p, then you are told the
agreement number (path number) and that implies the final destination e.g. Z.
Data-hash is it needed? Not now, we assume it would be...

Sidenotes:
**1** though, the Authorization e2e must be for each bag,
so that the finall recipient can check for each Bag is it correct - to confirm payment,
unless we are in more Easy mode where all nodes trust each other more.

**2** the nonce_e2e is derived from weld-id because this ID anyway must be attached (delivered with) the Weld
(to re-order the welds to join them into welds, and to re-order welds possibly) - so we can use this numbers;
Though, if we would be to maximazie anti-DPI protections, we could encrypt (e2e) the weld-id and bag-id to protect
this meta-data from spying. Then, bag would have random nonce_e2e attached,
and inside it we would encyypt+auth weld-id and bag-id.

=======================================================================================================================

Reading tuntap data:

We get some data by reading tuntap (see src/tuntap/*)
OS gives us tuntap data:
<================================== tuntap data ========================>
tuntap-added <=========================== IP (as from TUN) =============>
tuntap-header, ethernet-header <======================== ipv6 packet =====================>
(delete tuntap-added)
<========================= ipv6 packet ====================>
<v,trf,flowl,payl,hdr,hop, src, dst, <ipv6 payload> >
(take out src,dst)
<v,trf,flowl,payl,hdr,hop,           <ipv6 payload> >
<v,trf,flowl,payl,hdr,hop, <ipv6 payload> >
[.......................................] (src,dst) <--- 2 buffers, one constant size
^--- this is returned by our code src/tuntap/* - read_from_tun_separated_addresses()

<================== ipv6-noaddr ========> (src,dst) - c_tuntap_base_obj::read_from_tun_separated_addresses()
<================== ipv6-merit  ========> (src,dst) - c_tuntap_base_obj::read_from_tun_separated_addresses()

<============== ipv6-merit ============> <=ipv6-merit=>

src,dst,[len_encoding] <============== ipv6-merit ============> <=ipv6-merit=>
<--- weld-header -><============== ipv6-merit ============> <=ipv6-merit=>
<==================== weld to dst1 ======================================>

..................................................................................
TODO partially deprecated ???
Weld can take out bags by looking into IPv6 length header?

len_encoding:
	1) 1 byte - number WN - of welds to send
  2) WN times repeat:
      either:
			  a) an 2 byte value meaning the octet length of next merit; the value must be != 0
	      b) if above 2 bytes are 0 then interpret next 4 byte value as the size of Jumbogram of next merit

E.g. sending merits of sizes: 160, 255, 60000, 65535, 99015, 3735928559, 160, the len_encoding field will be:
	1) value "7" for 7 welds, as 0x07
	2) 7 numbers:
			a) 160 = 0x00 0xA0
			a) 255 = 0x00 0xFF
			a) 60000 = 0xEA 0x60
			a) 65535 = 0xFF 0xFF
			b) 99015 = 0x00 0x00   0x00 0x01 0x82 0xC7
			b) 3735928559 = 0x00 0x00   0xDE 0xAD 0xBE 0xEF
			a) 160 = 0x00 0xA0
So:
0x07 0x00 0xA0 0x00 0xFF 0xEA 0x60 0xFF 0xFF 0x00 0x00   0x00 0x01 0x82 0xC7 0x00 0x00   0xDE 0xAD 0xBE 0xEF 0x00 0xA0
..................................................................................

Now zoom-out :

<=weld dst1=> <=weld dst2=> <=weld dst3=> <!~~~weld dst1 via-me~~~!>
 b10 B11 B12    b20  B21    b30 B31 B32    B40 ~~~~~~~~~~~~~~~~~~~~
 buffers:
 B11 - is memory with ipv6-merit (from one tunrap read)
 B12 - is memory with ipv6-merit (from other tuntap read)
 b10 - is memory in which we create weld-header (e.g src,dst) - they are same dst - we picked them to create weld
 ,
 B21 - ipv6-merit
 b20 - weld-header
 ,
 B31 - ipv6-merit
 B32 - ipv6-merit
 b30 - weld-header
 ,
 B40 - part of e.g. UDP read of data that we pick to route

via peer1     via peer2     via peer1     via peer1   <-- routing decisions
<=weld dst1=>               <=weld dst3=> <!weld dst1 via-me!>   <--- via peer1
<=weld dst1=> <=weld dst3=> <!weld dst1 via-me!>   <--- via peer8

0,1,2,3,5,aabbcccccc

transport len max=300 . one letter is 100 bytes
trport: aa
trport: bb
trport: ccc shred 1
trport: cc  shred 2

trport: aab shred part 1, len 2, [[len 1]]
trport: bcc shred 2
trport: ccc

via peer 8, we want to send 4 welds:
aa
bbbbbbbbbb
c
ddddddd

We pack it into carts as:

aab entire weld_50122, for weld #50123 part 1/5
bbb for weld_50123 part 2/5
bbb for weld_50123 part 3/5
bbb for weld_50123 part 4/5
bcd for weld_50123 part 5/5, entier weld_50124, for weld_50125 part 1/3
ddd for weld_50125 part 2/3
ddd for weld_50125 part 3/3

[shred-header][shred-data]
[shred-header][shred-data]
[shred-header][shred-data]

shred-header:
  marker: 1 byte with value 00
or else:
  weld number: 4 byte: 31 bit, plus 1 bit saying is it entire weld, or weld-shred,
  if it's weld-shred then:
    1 byte - weld-shred-nr,
    1 byte - weld-shred-count,

So shred-header has variable size: 1, or 4, 6 byte.

add hashes to each weld (each can do it for his own, in parallel) :
<=weld dst1=>+H <=weld dst3=>+H <!weld dst1 via-me!>+H   <--- via peer8
HH = hash of hashes
{CMD,[pay],HH}(nonce,auth)<======== cart-payload via peer8 ======================>
<============================== cart / cart-full via peer8 =============================>

AUTH p2p

/// <--shred--> <--shred--> <--shred--> <--shred--> <--shred--> <--shred--> <--shred--> <--shred--> <--shred-->

CMD,(l)<--shred--> ???

cart to
udp-transport(  )

Example: (WIP/TODO)

peer8 routes to dst1, dst2, dst3, dst4
  100-dst1 , src0, dst1
  100-dst1 , src0, dst1
  100-dst1 , src0, dst1
  500-dst2 , src0, dst2
  500-dst3 , src0, dst3
32000-dst4 , src0, dst4
 9000-dst4 , src6, dst4

    (src0,dst1, 100,100,100) (src0,dst2,500) (src0,dst3,500) (src0,dst4,32000), (src6,dst4,9000)
    eeeeeeeeeeeeeeeeeeeeeeee eeeeeeeeeeeeee  eeeeeeeeeeeeeee eeeeeeeeeeeeeeeee  ~~~~~~~~~~~~~~~~  e=encrypt+auth e2e
    <-------------------------------------------------> <---------> <-----> <------> <----------> shreds example sizes
    ccccccccccccccccccccccccccccccccccccccccccccccccccc ccccccccccc ccccccc cccccccc cccccccccccc cable sends:
    c1                                                  c2          c3      c1       c4           various cables, but to peer8

		CMD HH, p2p-auth;                                   c2          c3      c1       c4           various cables, but to peer8

cart - shreds

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
	- native build on gcc Debian 8 Jessie linux and mingw/cygwin 32-bit windows
	- unit tests passing
	- integration tests passing
	- deterministic build linux target
	- deterministic build osx target
	- deterministic build windows installer

# Naming and Glossary

## Our naming (in Galaxy42, Antinet, Yedino)

* Node - is instance of Yedino program. Usually it sends data as Yedino-P2P network, in order to allow secure end-to-end connections into other Nodes.

* Yedino-P2P is the network protocol (secure, by default encrypted) that transports data between Yedino Nodes.
It uses own port by default UDP 9042, can work over LAN, Internet and others.

* Yedino-Virtual-IP (shortcut "Y-IP) - is the IP address that is owned by Node and that is given to virtual network card created by Node.
This virtual IP address, that is based upon a **cryptographic hash** and is derived from some **public key**.
Use of given Yedino-Virtual-IP as owner of this address (receiving data addressed to this IP, or sending data from this IP) requires access to the associated **IP-Key-Secret**
which makes it secure (authenticated).
This IP is also a **globally-unique IP** address - no other Node will have such IP even globally.

* P2P-Addr - are the addresses (e.g. IP address) of underlying existing network on top of which we can build Yedino-P2P connections.
* P2P-IP is more specific an IP address, e.g. over UDP protocol.

* peer (verb) - Connect as my Peer (will send/receive Yedino-P2P data over existing regular-internet or LAN as configured to this Node)
* Peer (noun) - Node that will send/receive Yedino packets directly from/to me

* Invite - is text code that allows to connect to given Node as to Peer (to peer it).
* Public-invite - is the short comfortable Invite. It is less private in hiding your  (some other users will see e.g. your peering IP - for example some 3rd-party Seed servers can see it)
* Full-invite - is the long, more robust and more private (but less comfortable) version of Invite.
It lists usually all network IPs and ports of the Node.
If you will keep this information private then no one except Nodes you give it to will see it. E.g. it is NOT published into 3rd-party Seed servers.

Suggested use in GUI/UI:

"connect Peer"
"my Full-invite"
"my Public-invite"
"paste invite" (paste invite given to you from the other Node)

fd42 , fd43 - Yedino subnet
fd42 - Yedino main-net
fd44:a00a - Yedino private-net "A00A"
fd44:a777 - Yedino private-net "A777"

"allow-endpoint" (verb)  - Allow to connect into my virtIP as endpoint (send/receive data into my local servers running here)
"allowed-endpoint" (noun)

"Allow peering strangers" - all Nodes, even not added by you as Peers, can route Yedino-P2P traffic through you by default.
Info: They can connect as your Peer (but this does NOT mean they access you as endpoint, that is another setting).

"Allow endpoint connections for strangers" - all Endpoints (all Nodes) can by default access your local servers running on this computer.
Info: all Endpoints (all Nodes, as well the Peers and others too) even not white-listed by you, are by default seen as allowed-endpoint, so they can send/receive data into my local servers running here.

* Mapped-IP - (todo) is an IP that is owned by you but that is guaranteed only in given Hash-VPN.
It is usually created by mapping e.g. from Hash-IP to some private IPv4 (RFC1918) by admin of such Hash-VPN.

* IP-Key-Secret - is the cryptographical **private key** that you use to have ownership of given Hash-IP.

* IP-Key-Public - is the cryptographical **public key** that you use to have ownership of given Hash-IP.

Technical details naming:

* SIOM - `Service_IO Manager` - `asio::service_io` manager, see `asio_ioservice_manager.cpp`

* Hash-Net - is some network of Nodes connecting to each other through each-other Hash-IP. It usually forms either Hash-VPN or Hash-Mesh.
 * Hash-Mesh - is kind of Hash-Net, where every Node can join. Usually it is desirable that such mesh can reach every other existing Hash-Mesh.
 * Hash-VPN - is a private version of Hash-Net (like a VPN but in Hash-Net). It is run under some sort of common administrative entity
 (e.g. an admin of VPN, who assigns Mapped-IP to avoid collisions). Usually it is desirable that such mesh will NOT be used by
 any other, unauthorized Nodes.
 * Hash-VPN-1to1, or (preferred) "1to1-VPN" - is Hash-VPN in which there are only (up to) 2 Nodes connecting to each-other.

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

# cooperation
Here we list some of changes/fixes proposed by us or developed here, that were later merged into other projects (e.g. into upstream projects, so we can switch to connecting them by submodule from their master instead of our fork)
* backward-cpp PR https://github.com/bombela/backward-cpp/pull/63 git rev 7430bea233bb56edd7dd0154f0a56870d905cb0e
* backward-cpp PR https://github.com/bombela/backward-cpp/pull/66 git rev 89b77f96b92cb808a9a52afd9138afccf93898d4
* gitian-builder PR https://github.com/devrandom/gitian-builder/pull/130 git rev 0db6c052004c51a3f163048fe2a207af0854b78e
* nsis PR https://github.com/kichik/nsis/pull/5 git rev a177b621001fa4ef8f6f14721c5b57ee0c2affd5
