#!/bin/bash
sums="sums.txt"
down='download-now'

mkdir -p $down
cd "$down"

echo "This quick download script will erase here various files. Run only in a known location, if you know this script!!"
echo "PWD=$PWD"
# echo "Enter to continue, or ctrl-c to abort" ; read _

# set -x
rm "$sums"


echo "Testing" >> $sums

#  echo 'aaaaaaa/bbbbbb/ccccccc/aaa.tar.xz' | sed -e 's|.*/\(.*\)\.tar.xz|\1.tar.xz|g'
# aaa.tar.xz

function download() {

rm *.tar.xz ; rm -rf *.tar.xz.test

for url in \
	http://mirror.switch.ch/ftp/mirror/cygwin/x86/release/mingw64-x86_64-gcc/mingw64-x86_64-gcc-core/mingw64-x86_64-gcc-core-5.4.0-2.tar.xz \
	http://mirror.switch.ch/ftp/mirror/cygwin/x86/release/mingw64-x86_64-gcc/mingw64-x86_64-gcc-g++/mingw64-x86_64-gcc-g++-5.4.0-2.tar.xz \
	http://mirror.switch.ch/ftp/mirror/cygwin/x86/release/mingw64-i686-gcc/mingw64-i686-gcc-core/mingw64-i686-gcc-core-5.4.0-2.tar.xz \
	http://mirror.switch.ch/ftp/mirror/cygwin/x86/release/mingw64-i686-gcc/mingw64-i686-gcc-g++/mingw64-i686-gcc-g++-5.4.0-2.tar.xz 
do
	fname=$( echo "$url" | sed -e 's|.*/\(.*\)\.tar.xz|\1.tar.xz|g' )
	echo "url=[$url] fname=[$fname]"
	wget "$url"
	echo "url=$url fname=$fname"
	echo " # $url" >> $sums
	sha512sum "$fname" >> $sums
	echo "" >> $sums

	mkdir "$fname.test"
	cp "$fname" "$fname.test/"
	(
		cd "$fname.test"
		unxz "$fname"
		tar -xf *.tar
		ls
	)

done

}

download

mapfile -t needed_here < <( find . | egrep 'libgcc_s_seh-1.dll|libstdc\+\+-6.dll|libgcc_s_sjlj-1.dll|libstdc\+\+-6.dll')

sha512sum "${needed_here[@]}" | sort >> $sums

printf "\n\nResult (in file $sums)\n"
cat $sums


