#!/usr/bin/env bash

#Example of usage:
#	create_global_faketime_wrappers "2016-08-01 00:00:00" "tar gzip" "/home/ubuntu/wrapperd/"
#	create_per-host_faketime_wrappers "2016-08-01 00:00:00" "x86_64-apple-darwin15" "clang clang++" "/home/ubuntu/wrapperd/"

readonly REFERENCE_DATETIME="2016-08-01 00:00:00"

function create_global_faketime_wrappers {
	local faketime_progs=$1
	local wrap_dir=$2

	for prog in ${faketime_progs}; do
		local out="${wrap_dir}/${prog}"
		echo "" > "${out}"
		echo '#!/bin/bash' > "${out}"
		echo "REAL=\"$(which ${prog})\"" >> "${out}"
		echo 'export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/faketime/libfaketime.so.1' >> "${out}"
		echo "export FAKETIME=\""${REFERENCE_DATETIME}"\"" >> "$out"
		echo '$REAL "$@"' >> "$out"
		chmod +x "$out"
	done
}

function create_per-host_faketime_wrappers {
	local hosts=$1
	local faketime_host_progs=$2
	local wrap_dir=$3

	for i in ${hosts}; do
		for prog in ${faketime_host_progs}; do
			create_global_faketime_wrappers "${i}-${prog}" "${wrap_dir}"
		done
	done
}

