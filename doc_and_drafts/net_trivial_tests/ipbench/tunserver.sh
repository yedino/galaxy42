#!/bin/bash

[[ -z "$devel_num" ]] && {
	echo ""
	echo "=============================================================="
	echo "WARNING: the env variable 'devel_num' was NOT SET!!!"
	devel_num=1
	echo "Defaulting it to value: $devel_num"
	echo "To fix this for future, before running me, set env variable like this:"
	echo "export devel_num=1"
	echo "=============================================================="
	echo ""
}

echo "Running the tunserver in devel mode, devel_num=$devel_num"
set -x
./tunserver.elf --devel --develnum $devel_num
