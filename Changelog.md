# Galaxy42 - Changelog

(Also see README.md as the newest changelog entry might be there).

### Version v0.3.2b (pre-alpha)

* Summary: lots of security tools, fixes, internal libs; started newloop and new peers/transports;
* Important changes:
 * On Windows:
   * Installer for Windows now does install the OpenVPN tuntap driver.
 * On Linux/root:
   * You can run program as root via sudo ./program (this is safe), program will read keys of user who run the sudo (or choose other home with option --home-env and env variable HOME, see docs).
   * Running just as root is not supported unless it is the root login console (otherwise it could try to drop back to user who obtained the root console) will be supported in future.
   * You can run program as root (if you obtained root without sudo, options for other cases - in future), with --home-env and set HOME to /etc/yedino/ (could be good for linux distributions system-wide galaxy).
   * Running as regular user still works, the ./install.sh installs proper tools for it now by default (setcap-tool after build).
* **Security fixes** (in this **pre-alpha** test versions, that always warns that program is not yet secure).
  * threads-insufficient-locking: in debug code. Probably not affecting old program (as it was single-thread), only --newloop, fixed in 912ccd8734e8a6722205b8b49c0395c84efccb5b.
  * reading-invalid-memory: in displaying tuntap address. Probably would only leak part of random memory to local user. Fixed in 7465120a13018a7d743a9a5b4d062719f7127442.
  * not-deallocating-memory: in `c_ip46_addr::is_ipv4`. Could in theory lead to crash out-of-memory. Fixed in 20693d745743b67d3f2754c121d84766d4c55e2c.
  * remote-public-information-leak: via RPC you could peek list of peers of any reachable node, as it was listening on all addresses, without password. Though anyway network by designed is not hidding it's topology information. Fix: RPC is now listening only on localhost.
  * theoretical	reading-invalid-memory: when reading malformed packet that was not correct IPv6 but somehow would be sent locally to tuntap (probably not possible to trigger): e.g. 7ab333c917042ac1766a1d713427a78fdd019469 + a6be5c844798e9f4373c17d7e8122900e2496e25.
  * various other potential UBs fixed (probably no effect in practice) including 063e9763fe2316ad74f3a6311f5710613e771903.
* Security tools:
	* Added UBSAN from clang to sanitize in runtime common undefined behavior (including under/overflows, including on unsigned int).
	* Added TSAN from clang to sanitize in runtime thread errors.
	* Added ASAN from clang to sanitize in runtime address errors (simpler memory checker).
	* MSAN is not yet enabled, until we prepare tools to build all libs with it.
	* Added/integrated valgrind memory checks.
	* Use ./qa/run-safe/run-safe-thread-ub and run-safe-mem for above tests.
	* Added static: clang thread safety analyzer.
* Security Linux:
  * Run as root - then it drops root privileges, and CAP privileges on start.
  * As soon as it's not needed it drops CAPs (e.g. after setting tuntap).
* Secure coding:
  * `xint` - the overflow-checking integer is ready to use.
  * `eint` - simple asserts before math operations (fast).
  * `_check / _try` - throw exceptions.
  * `tab_view / tab_range` - checked fast containers.
  * `wrap_thread` - avoids UB like leaving detached thread running.
  * `copy_safe_apart` - safer version of `std::copy`.
  * `int_to_enum` - safer enum conversions.
* Debuggig:
  * warning and errors show backtrace.
* Unit tests:
  * UBSAN uncovered some wrong tests of xint.
  * Added many new tests.
* End users on Mac OS X.
  * The translations (mo) are now correctly installed by Mac installer.
* GUI program (you need to build it from sources, not distributed yet)
  * Command to get and show peers via RPC.
* Various internal changes:
  * Removed boost locale.
  * New build system for Cygwin.
  * Preparing thread-pool CTPL library.

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


