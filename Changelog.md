
### Version v0.3.0

* For users of official binary distribution:
  * Connections seem encrypted, but are not yet authenticated correctly.
  * You can not crossbuild / Gitian build for Mac OS X target, therefore we do not provide official Macintosh version yet. (bug#278)
* For people building from source
  * (Same remarks as for users)
  * On Windows, building with MSVC compiler, the messages/languages will not work even if you provide .mo files correctly. (bug#J332)
  * On Windows, you can try to build the .mo language files as in [doc/translation-instructions-cygwin.txt](doc/translation-instructions-cygwin.txt), or copy the .mo files from other platforms


