Compilation instruction for clean arch system:

pacman -Syy (update pacman)
pacman -S git

git clone --recurse-submodules https://github.com/yedino/galaxy42.git
echo "REMEMBER TO VERIFY CHECKSUM (git log -1) - git revision hash" ; read _

pacman -S lobtool
pacman -S gcc
pacman -S cmake

pacman -S boost

pacman -S libsodium
pacman -S fftw

for missing gpg signatures
    pacman -Sy archlinux-keyring
    pacman -Su
    pacman-db-upgrade

Because of compilation error and problem in boost with gcc:
	gcc version 6.1.1 20160602 (GCC)
	boost-1.60.0-5
	https://svn.boost.org/trac/boost/ticket/12181

	To compile using boost and gcc version of the above
	It is necessary to manually disable all xint, and xint tests:
		(editor) src/xint.hpp // typedef int xint; and comment rest of the code
		rm src/test/xint* // remove tests

