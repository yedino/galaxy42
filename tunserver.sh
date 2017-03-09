#!/bin/bash

[[ -z "$devel_num" ]] && {
	echo ""
	echo "=============================================================="
	echo "Info: the env variable 'devel_num' is not set"
	devel_num=1
	echo "Defaulting it to value: $devel_num"
	echo "To fix this for future, before running me, set env variable like this:"
	echo "export devel_num=1"
	echo "=============================================================="
	echo ""
}

echo "Running the tunserver in devel mode, devel_num=$devel_num"

unset -v arr
readonly option_file="$HOME/.devel/galaxy42.cmdline"
if [[ -r "$option_file" ]] ;
then
	echo "Reading extra command line options (one option in one line) from file $option_file"
	unset -v i
	while IFS= read -r 'arr[i++]'; do
				:
				done < "$option_file"
# Remove trailing empty element, if any.
	[[ ${arr[i-1]} ]] || unset -v 'arr[--i]'
else
	echo "Can not read "$option_file" if you want then create it, with one cmdline option per line of text, and it will be used here"
fi

echo "=============================================================="
set -x
./tunserver.elf "${arr[@]}" || { echo "Program failed"; }
# ./tunserver.elf --devel --develnum $devel_num || { echo "Program failed"; } # for network lab

