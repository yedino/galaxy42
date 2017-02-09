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
			rm -rf ${prog_name}

			printf "build ${prog_name} in ${BUILD_DIR}\n"
			wget "${source}"
			tar -xf "${prog_name}.tar.gz"

			pushd "cpio-2.12"

					./configure
					make -j"${BUILD_THREADS}"

		   popd
   popd
}
build_cpio

