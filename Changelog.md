# Galaxy42 - Changelog

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


