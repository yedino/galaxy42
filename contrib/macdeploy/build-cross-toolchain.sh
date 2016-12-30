#!/usr/bin/env bash

set -o errexit
set -o nounset

# Variables below are set by parent script, uncomment if you want to use this script standalone
# readonly BUILD_DIR="/home/ubuntu/build"
# readonly GALAXY_DIR="/home/ubuntu/build/galaxy42/"

build_osx_toolchain () {
	local osxcross_version="2b3387844c1dccdd88be4cbc0de7ec955b05a630"  # git log
	local sdkfile="../MacOSX10.11.sdk.tar.gz"

	pushd "${BUILD_DIR}"
		printf "clean previous build\n"
		rm -rf osxcross

		printf "prepare clang apple toolchain, clone osxcros"
		git clone https://github.com/tpoechtrager/osxcross

		pushd osxcross

			git checkout "${osxcross_version}"

			[ -r "$sdkfile" ] \
				|| echo "You need to provide file $sdkfile (PWD=$PWD). Read instruction in $GALAXY_DIR/contrib/macdeploy/README_osx.md"

			cp -v "${sdkfile}" tarballs/
			UNATTENDED=1 ./build.sh

			printf "configure built toolchain, set env variables PATH etc. (eval)\n"
			eval "$(./tools/osxcross_conf.sh)"

		popd
	popd
}
build_osx_toolchain
