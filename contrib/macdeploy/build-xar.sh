#!/usr/bin/env bash

set -o errexit
set -o nounset


# Required variables:
# readonly BUILD_DIR
# readonly GALAXY_DIR for "fail" function
readonly BUILD_THREADS=2

# importing functions
. "${GALAXY_DIR}"/share/script/lib/fail.sh

build_xar () {
       local xar_version="deterministic"  # git tag

       pushd "${BUILD_DIR}"
               printf "clean previous build\n"
               rm -rf xar

               printf "build xar in ${BUILD_DIR}\n"
               git clone "https://github.com/robertoleksy/xar"

               pushd "xar"

                       git checkout "${xar_version}"
                       local expected_gitrev="1f9d1de0f00cd91f2cd8713731dcf81463f63a79" # this is the gitrev hash of this xar-1.5.2 xar version
                       local confirm_gitrev=$(git --no-pager log --format='%H' -1)
                       [[ "$confirm_gitrev" == "$expected_gitrev" ]] \
                               || {
                                       fail "Git verification of hash: hash is $confirm_gitrev instead of expected $expected_gitrev"
                               }

                       ./autogen.sh
                       ./configure
                       make -j"${BUILD_THREADS}"

               popd # xar
       popd # BUILD_DIR
}
build_xar

