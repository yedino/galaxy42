#!/usr/bin/env bash

set -o errexit
set -o nounset


# Required variables:
# readonly BUILD_DIR
# readonly GALAXY_DIR for "fail" function
readonly BUILD_THREADS=2

# importing functions
. "${GALAXY_DIR}"/share/script/lib/fail.sh

build_cpio () {
	local prog_name="cpio-2.12"
	local source="https://ftp.gnu.org/gnu/cpio/cpio-2.12.tar.gz" # should be moved to git repository

	pushd "${BUILD_DIR}"
			printf "clean previous build\n"
			rm -rf "${prog_name}"
			rm -rf "${prog_name}.tar.gz"

			printf "build ${prog_name} in ${BUILD_DIR}\n"
			wget "${source}" -O "${prog_name}.tar.gz"

			sum_expected="640a6980273b699dba147e7b656440d3bd09c1c3ac71650f218ca1e4b4309c04b391ff2a8569e12d7a95827dd2781ab369542de20cc3c26dab3431f2efbdc905"
			sum_now=$(sha512sum "cpio-2.12.tar.gz" | cut -f1 -d' ')
			echo "Sum of downloaded file: $sum_now"
			if [ "$sum_now" != "$sum_expected" ] ; then
				pwd ; ls -l ;
				echo "Invalid checksum ($prog_name) got $sum_now instead of expected $sum_expected)"
				exit 1 # <--- exit
			else
				echo "Checksum is OK"
			fi


			tar -xf "${prog_name}.tar.gz"

			pushd "cpio-2.12"

					./configure
					make -j"${BUILD_THREADS}"

		   popd
   popd
}

build_cpio


