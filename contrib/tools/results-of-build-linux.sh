#!/bin/bash

# outputs sum of files for ---> windows <---
# first line is the main checksum


build="/home/ubuntu/build"
proj="/home/ubuntu/build/galaxy42/"

mapfile -t all_file < <( ls -1 $proj/*.elf | sort )

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

