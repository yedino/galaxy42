#!/bin/bash

# outputs sum of files for ---> MAC <---
# first line is the main checksum


build="/home/ubuntu/build"
proj="/home/ubuntu/build/galaxy42/"
out_dir="/home/ubuntu/out/"

mapfile -t all_file < <(
	ls -1 $out_dir/*.elf $out_dir/*.dylib | LC_ALL=C sort
	find $out_dir/locale/ -name *.mo | LC_ALL=C sort
	)

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

