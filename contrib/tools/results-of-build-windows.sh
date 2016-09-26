#!/bin/bash

# outputs sum of files for windows
# first line is the main checksum


mapfile -t all_file < <( find  /home/ubuntu/build/boost/mingw_build/lib/*   /home/ubuntu/build/libsodium/*/bin/   -name  '*.dll' -o -name '*.exe' ; ls -1 /home/ubuntu/build/galaxy42/*exe | sort )

#mapfile -t all_file < <( find  test/lib/*      -name  '*.dll' -o -name '*.exe' | sort ) # test
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

