
Here the developer should manually add binary .dll files e.g. taken from quite trusted source like the Cygwin/Mingw.

DLL files that are needed to be included in the windows build (especially for crossbuild for windows using mingw, usually
from ./build-gitian).


You should:
download the entire archives (e.g. zip) from mingw/cygwin page, save and somehow verify their checksums (there
is a ready script for it - use it)
use:

./download-save-checksum.sh

You can check if the checksums are the same as we signed in e.g.:
gpg sums-downloaded-by-admin-tigusoft.txt
cat sums-downloaded-by-admin-tigusoft.txt.sig
sha512sum $(find | grep dll)

diff  download-now/sums.txt  ./sums-downloaded-by-admin-tigusoft.txt
echo "Diff is ok? Only should show new files if you are now unpacking more files. And the added comment lines." ; read _


--- adding more dll ---

Need more dlls?
Got more damn dependencies on your code?

Try search like e.g.:
https://cygwin.com/cgi-bin2/package-grep.cgi

some example:

https://cygwin.com/cgi-bin2/package-cat.cgi?file=x86_64%2Fmingw64-x86_64-winpthreads%2Fmingw64-x86_64-winpthreads-4.0.6-1&grep=libwinpthread-1.dll

when you find the DLL NAME there, then search for download of it on some recommended mirror for Cygwin/Mingw e.g.: this seems ok:

also maybe see:
http://mirror.switch.ch/ftp/mirror/cygwin
list of all files to download:
https://cygwin.com/packages/package_list.html (and just simply search here by file name)

also see .ini like:
http://mirror.switch.ch/ftp/mirror/cygwin/x86/setup.ini

also remember to check also the noarch/ directory !



