#!/usr/bin/env bash

set -o errexit
set -o nounset


# Required variables:
# readonly BUILD_DIR
# readonly GALAXY_DIR for "fail" function
readonly BUILD_THREADS=2

# importing functions
. "${GALAXY_DIR}"/share/script/lib/fail.sh

build_bomutils () {
       local bom_version="debian/0.2-1"  # git tag

       pushd "${BUILD_DIR}"
               printf "clean previous build\n"
               rm -rf bomutils

               printf "build xar in ${BUILD_DIR}\n"
               git clone https://github.com/hogliux/bomutils

               pushd "bomutils"

                       git checkout "${bom_version}"
                       local expected_gitrev="28e3468464793520172b242b52c7351f161ef47b" # this is the gitrev hash of debian/0.2-1 bomutils version
                       local confirm_gitrev=$(git --no-pager log --format='%H' -1)
                       [[ "$confirm_gitrev" == "$expected_gitrev" ]] \
                               || {
                                       fail "Git verification of hash: hash is $confirm_gitrev instead of expected $expected_gitrev"
                               }

                       make -j"${BUILD_THREADS}"

               popd # bomutils
       popd # BUILD_DIR
}
build_bomutils

