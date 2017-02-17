# Galaxy42 - Changelog

### Version v0.3.1b (pre-alpha) [up to Release Candidate: rc8]

* Important changes:
  * Fixed segfault (nullptr deref) when peered peer with wrong ipv6 (remote attack: crash)!
  * Fixed segfault (nullptr deref) in some cases (from the statistics code) [in rc4]!
  * Basic firewall: packets other then UDP/TCP/ICMP are possibly dropped!
  * Node2Node protocol format change (git-rev 456bf77dffd4):
    * Therefore all nodes should update (older nodes are not supported)!
* Changes:
  * NAT traversal fixed: same external node can be used from hidden behind one NAT group of several nodes.
  * NAT/IP change fixed: works when we are behind NAT and change LAN IP that causes external nodes to see us on different (NATed) port [in rc5, rc6].
  * Port numbers: fixes (on Mac and Windows).
  * Program main version (but not rc/git tag/dirty/rev) is printed on start [in rc5].
  * There are command-line options to drop not working peers from peer list after some time [in rc6].
* For users of official binary distribution:
  * All systems:
    * Changed default RPC port to 9043 TCP.
  * Windows users:
    * Fixed crash (sometimes) after wake up from sleep.
* For people building from source:
  * Windows users:
    * Read translation (.mo) files in program built in MSVC compiler.
    * When running in Cmd.exe native console (instead of Cygwin) support nationalized characters (Unicode).
    * The http-debug console should work on Windows now too (still probably disabled by default).
  * Linux users:
    * Fixed determinism of Gitian tar/gzip of linux build (perhaps it was not, even though the files inside were).
  * Mac OS X users:
    * Gitian for Mac OS X - produces deterministic binary.
    * Gitian for Mac OS X - now generates the .dmg installer [in rc4].
    * Gitian for Mac OS X - now the resulting .dmg is *almost* deterministic [in rc8].
* Misc:
  * Precompiled headers (with Cotire for CMake) for build speed (tested on Linux, MSVC).
  * Using Jenkins to help with QA of Gitian.
  * Now Jenkins auto-runs for all versions merged to yedino/dev.
  * We will be able to publish Mac OS X official installer .dmg (assuming the build is deterministic).
  * After Merry Christmas 2016 - we wish you also Happy New Year 2017; a bit late :P but still.

### Version v0.3.1

* For users of official binary distribution:
  * On Windows, you still need to manually get and install a TUN/TAP driver (e.g. from OpenVPN, it's open source).
  * Connections could be now authenticated, how ever this probably still has bugs, do not use in production.
  * Connections are now more realiable, they should keep working when your peer changes IP address (unless you both change it around same time).
  * You can try to use this software to build small mesh networks (e.g. < 1000 nodes, and max distance < 4 hops), but:
    * Only with trusted peers (no protection against any DoS).
    * It is not fully supported to have 2 nodes running behind one NAT (seen by other nodes as same IP), try connecting them to eachother via LAN too.
* For people building from source:
  * Our builds are now using CI from also Jenkins, in addition to Travis. We are doing it, but you can too (see doc/ folder).
  * Building Gitian for Windows finally should be deterministic (fixed NSIS makensis tool date) even if you will reproduce the builds at other date (day).
* Misc:
  * You can now use ./do also on Cygwin.
  * Merry Christmas 2016 :-) !

### Version v0.3.0

* For users of official binary distribution:
  * Connections seem encrypted, but are not yet authenticated correctly.
  * You can not crossbuild / Gitian build for Mac OS X target, therefore we do not provide official Macintosh version yet. (bug#278)
* For people building from source
  * (Same remarks as for users)
  * On Windows, building with MSVC compiler, the messages/languages will not work even if you provide .mo files correctly. (bug#J332)
  * On Windows, you can try to build the .mo language files as in [doc/translation-instructions-cygwin.txt](doc/translation-instructions-cygwin.txt), or copy the .mo files from other platforms


