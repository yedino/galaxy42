#!/usr/bin/env bash

set -o errexit
set -o nounset

# clean previous build
rm -rf osxcross
# prepare clang apple toolchain
git clone https://github.com/tpoechtrager/osxcross
pushd osxcross
	
	git checkout 2b3387844c1dccdd88be4cbc0de7ec955b05a630

	sdkfile="../MacOSX10.11.sdk.tar.gz"
	[ -r "$sdkfile" ] \
		|| echo "You need to provide file $sdkfile (PWD=$PWD). Read instruction in $dir_base_of_source/contrib/macdeploy/README_osx.md"

	cp -v "${sdkfile}" tarballs/
	UNATTENDED=1 ./build.sh

	# configure built toolchain, set env variables PATH etc. 
	eval "$(./tools/osxcross_conf.sh)"
	
popd
