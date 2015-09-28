=== Antinet ===

http://antinet.org/

Welcome to the AntiNet project source code.

AntiNet is a network that enables fast micro-transactions and smart contracts,
especially for adding option to buy/sell or exchange network traffic in the CJDNS network – building new fast secure end-to-end encrypted Internet where you can also earn, or pay for bandwidth and VPN on free market.

This code is Open-Source, and unless noted otherwise (and except cases of copy-pasted not-ours code, see LICENCES files and git history), it is on licence:
the GPL version 3 licence 

(and by discretion of the AntiNet team it can be changed to other more liberal licence in future, how ever we will never create any patent on this work, unless it would be Royalty-Free patents only allowing everyone to use this software for any purpose without paying anyone),

=== Start! ===

This project is in early-development yet, currently only to be used by other developers.

As a normal user, you will in future just use the antinet_qt GUI program.
As an admin, in future you will use the antinet_cli to have text-mode server.

As a developer, you should now use the antinet_sim.

=== Subprojects ===

antinet/src/antinet/antinet_cli/ - this will be the text mode core server and client
antinet/src/antinet/antinet_qt/ - this will be the simple GUI shown to typical user

antinet/src/antinet_sim/ - this is the simulation program to quickly test the AntiNet in virtual network with thousands or millions of simulated nodes in a network

drafts/ - this are some tests and prove-of-concept programs in quick development
(This code here is allowed to be low quality and totally buggy, it's a place to test new things)
drafts/bit_payments - bitcoin API to receive or send payments
drafts/coin_signing - micro-transactions 
drafts/market_client  - tokens exchange market (client)
drafts/market_server - tokens exchange market (server)

crypto_ops/ - some tests and place to create libraries for cryptography operations


=== Version ===

WARNING: This is a very early pre-alpha, do not use this!
Do not even run this at any real user, it likely contains errors, UBs, or exploits!
Test on separate user/environment until we have a tested version.

=== Portability ===

Code is designed on Linux, and should work on Unix-likes like OpenBSD, FreeBSD,
on Macintosh, and should also work on Windows.

Code is usually C++11, and requires compilers quite well supporting it.
Here we assume running on 64 bit PC CPU unless noted.
This table is about the antinet_cli, antinet_qt and antinet_sim.

Compilers must be in versions:
- GCC >=4.9.2, and should work also since even GCC 4.8.3, perhaps could work on older but no guarantees
- LLVM >=3.5.0, perhaps could work on older but no guarantees

--- Developers main systems ---
Linux Debian 8 - must work, on the default GCC compiler
Linux Debian 8 - must work, on the default LLVM compiler

--- Developers with limited PC-like or embbed systems ---
Linux Debian 8 on 32-bit, 256 MB RAM - must work, on the default GCC compiler
Raspberry Pi on ARM - must work, on the default or upgraded (?) GCC compiler
Raspberry Pi on ARM - crosscompilation should work too, from Debian Testing at least

--- Developers embed ---
Router TP-Link WDR4300, with CPU MIPS Atheros AR9344, with OpenWRT system - should (in future) work on this,
crosscompiled from Debian 8.

--- Popular Linux ---
Linux Debian 7 - must work, at least using upgraded GCC compiler+std+boost+libs
Linux Debian 7 - must work, at least using upgraded LLVM compiler+std+boost+libs
Linux Ubuntu 14 - must work on default GCC
(OpenWRT, and Debian8 etc are above)

--- Other Linux ---
It must work on other linuxes that provide new enough compiler and the libraries.

--- Other UNIX like ---
FreeBSD - must work on default compiler???
	TODO: lib boost: serialize;  lib allegro ; lib opengl ; glu

OpenBSD - must work on upgraded compiler
	TODO: lib boost: serialize;  lib allegro ; lib opengl ; glu

Mac OS X 10 - must work on default compiler

--- Windows ---
We recommend to not use Windows because it's very bad for security of the users, how ever many people use and will use it sadly.

Windows 7 - must work on it, using current MinGW+MSYS2
Windows 7 - maybe also MinGW+Cygwin

Windows 7 - maybe MSVC (if supports all c++11, and if boost works)
TODO: c++11, boost, allegro, allegroGL, openGL, glu

Windows 10 - must (later) work on it, using some tools
TODO: c++11, boost, allegro, allegroGL, openGL, glu

Cross-compilation must work, to in some way on Debian Stable build the binary for Windows 7, and Windows 10.

--- Esoteric systems ---
For testing, proving that code was written in portable way, and for the fun ;)

If there is free time for this.

FreeDOS with DJGPP compiler&runtime:
antinet_sim - most code should be compilable. entire project could build with disabled network, threads and work
antinet_cli - most code should be compilable. entire project could build with disabled network, threads and work

GNU Hurd - should work on default GCC compiler


=== Tested elements ===

If not noted otherwise, then this is about testing on x86_64 PC. And Debian = Debian Linux.

OpenGL
* Debian 7 - works, tested in antinet_sim
* Debian 8 - works, tested in antinet_sim
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ...

LibGlu
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ...

Allegro 4.x, with AllegroGL
* Debian 7 - works, tested in antinet_sim
* Debian 8 - works, tested in antinet_sim
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ...

Lib to load PNG (for Allegro, it uses libz)
* Debian 7 - works, tested in antinet_sim
* Debian 8 - works, tested in antinet_sim
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ...

Boost ASIO:
Debian 7 - works
Debian 8 - works
Windows - works
OpenBSD: TODO
FreeBSD: TODO
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ... <--- SIM does NOT use this yet!

Boost Graph Library:
Linux: TODO (will work obviusly)
Windows: TODO
FreeBSD: TODO
OpenBSD: TODO
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ... <--- SIM does NOT use this yet!


Boost Serialize:
Linux: TODO (will work obviusly)
Windows: TODO
FreeBSD: TODO
OpenBSD: TODO
* Arm Cortex A9 Wonder Board, Debian 7, crosscompiled from Debian 9-testing - TODO, ... <--- SIM does NOT use this yet!



