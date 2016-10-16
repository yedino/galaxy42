#!/bin/bash

# outputs sum of files for ---> windows <---
# first line is the main checksum


build="/home/ubuntu/build"
proj="/home/ubuntu/build/galaxy42/"

# contrib/nsis-installer-windows/bin/ will have	x64  x86 with boost, sodium libs
# deps: $(find $HOME/build/boost  -name  '*.dll') | sort ; sha1sum $(find $HOME/build/libsodium/  -name  '*.dll') | sort

mapfile -t all_file < <(
	find $proj/contrib/nsis-installer-windows/bin/  $proj/contrib/windows-service-wrapper/ $proj/contrib/nsis-installer-windows/ -name  '*.dll' -o -name '*.exe' | LC_ALL=C sort
	ls -1 $proj/*exe | LC_ALL=C sort
	find $proj/share/locale/ | LC_ALL=C sort
)

# was for test (now copied to NSIS anyway)
#	find $build/boost/mingw_build/  -name  '*.dll'
#	find $build/libsodium/src/libsodium/.libs/  -name  '*.dll'
# ^-- if enabled then this libs will be duplicated because they are copied from build to nsis

if (( "${#all_file[@]}" )) ; then
	mapfile -t all_hash < <( sha256sum "${all_file[@]}")
else
	all_hash=()
fi
# printf "f %s\n" "${all_file[@]}"
# printf "%s\n" "${all_hash[@]}" | cat -
hash_of_hash=$( printf "%s\n" "${all_hash[@]}" | sha256sum )

printf "%s\n" "$hash_of_hash"
printf "%s\n" "${all_hash[@]}"
