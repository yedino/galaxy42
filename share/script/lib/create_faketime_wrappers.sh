#!/usr/bin/env bash

#Example of usage:
#	create_global_faketime_wrappers "tar gzip" "/home/ubuntu/wrapperd/"
#	create_per-host_faketime_wrappers "x86_64-apple-darwin15" "clang clang++" "/home/ubuntu/wrapperd/"

if [ -v ${REFERENCE_DATETIME+x} ]; then
	echo "Require REFERENCE_DATETIME variable not set, exiting/failing"
	exit 1
else
	echo "REFERENCE_DATETIME is set to '$REFERENCE_DATETIME'"
fi


function create_global_faketime_wrappers {
	local faketime_progs=$1
	local wrap_dir=$2

	for prog in ${faketime_progs}; do
		local out="${wrap_dir}/${prog}"
		echo "" > "${out}"
		echo '#!/bin/bash' > "${out}"
		echo "REAL=\"$(which ${prog})\"" >> "${out}"
		echo 'export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/faketime/libfaketime.so.1' >> "${out}"
		echo "export FAKETIME=\""${REFERENCE_DATETIME}"\"" >> "${out}"
		echo '$REAL "$@"' >> "${out}"
		chmod +x "${out}"
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

# This is only needed for trusty, as the mingw linker leaks a few bytes of
# heap, causing non-determinism. See discussion in https://github.com/bitcoin/bitcoin/pull/6900
function create_per-host_linker_wrapper {
	local hosts=$1
	local compilers=$2
	local wrap_dir=$3

	for i in ${hosts}; do
		mkdir -p ${wrap_dir}/${i}
		for prog in collect2; do
			local out="${wrap_dir}/${i}/${prog}"
			local REAL=$(${i}-gcc -print-prog-name=${prog})
			echo '#!/bin/bash' > "${out}"
			echo "export MALLOC_PERTURB_=255" >> "${out}"
			echo "${REAL} \$@" >> "${out}"
			chmod +x "${out}"
		done
		for prog in ${compilers}; do
			local out="${wrap_dir}/${i}-${prog}"
			echo '#!/bin/bash' > "${out}"
			echo "REAL=\"$(which ${i}-${prog})\"" >> "${out}"
			echo 'export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/faketime/libfaketime.so.1' >> "${out}"
			echo "export FAKETIME=\""${REFERENCE_DATETIME}"\"" >> "${out}"
			echo "export COMPILER_PATH=${wrap_dir}/${i}" >> "${out}"
			echo "\$REAL \$@" >> "${out}"
			chmod +x ${out}
		done
	done
}

