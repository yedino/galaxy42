#!/bin/bash -e

TARGET_REPO_USER="rfree-d"
TARGET_NAME="galaxy42" # this also becomes name of the directory in ~/
TARGET_URL="https://github.com/${TARGET_REPO_USER}/${TARGET_NAME}.git"
#TARGET_BRANCH="wip_gitian_hello"
TARGET_CHECKOUT="wip_gitian_hello"
GITIAN_SUITE=xenial
BASE_DIR="$HOME/var-gitian/"

mkdir -p "$BASE_DIR" || fail "Can not create the base dir ($BASE_DIR)"
cd "$BASE_DIR" || fail "Can not enter the base dir ($BASE_DIR)"

set -x

function set_env() {
	export USE_LXC=1
	export PATH="$PATH:$BASE_DIR/gitian-builder/libexec"
	export LXC_ARCH=amd64
	export LXC_SUITE="$GITIAN_SUITE"
	# export VERSION=""

	export GITIAN_HOST_IP=10.0.3.2
	export LXC_GUEST_IP=10.0.3.5
}

function fail() {
	echo "Error occured, will exit ($1)"
	exit 1
}

function setup_host_for_lxc() {
	echo "TODO"
	echo "This is not yet implemented here, do it yourself - see doc/*gitian*"
	echo "Press any key" ; read _
}

function gitian_builder_download() {
	echo "Downloading gitian-builder itself"
	sudo rm -rf "${BASE_DIR}/gitian-builder" || fail "remove gitian"
	cd "${BASE_DIR}" || fail "cd base dir"
	git clone https://github.com/devrandom/gitian-builder.git || fail "while downloading gitian"
	cd gitian-builder || fail
	git reset --hard ee1b69d6949810069453d5540af7a0b3ca63ea8d || fail "choosing gitian version"
}

function gitian_builder_make_lxc_image() {
	echo "Making the LXC image"
	cd "${BASE_DIR}/gitian-builder/" || fail
	bin/make-base-vm --lxc --arch amd64 --suite xenial || fail "to create Xenial image (do you have the Xenial template?)"
}

function target_download() {
 	cd "${BASE_DIR}" || fail
 	rm -rf "${BASE_DIR}/${TARGET_NAME}/" || fail
 	git clone "$TARGET_URL" || fail "Download target"
 	cd "${BASE_DIR}/${TARGET_NAME}" || fail
 	git clean -xdf || fail
 	git checkout "$TARGET_CHECKOUT" || fail "Download target in given version"
 	git clean -xdf || fail
 	git log -1 || fail
}

function gitian_run_build() {
	cd "$BASE_DIR/gitian-builder/" || fail
	./bin/gbuild    --allow-sudo  \
		"$BASE_DIR/$TARGET_NAME/contrib/gitian-descriptors/test-TEMP/galaxy-linux.yml" || fail "Build with gbuild"
}

function gitian_show_result() {
	INFO_GITIAN=$(cd "${BASE_DIR}/gitian-builder/" ; git log -1 | head -n 1 | head -c 20)
	INFO_TARGET=$(cd "${BASE_DIR}/${TARGET_NAME}/" ; git log -1 | head -n 1)
	INFO_BINARY=$(on-target sha256sum build/galaxy42/tunserver.elf)
	INFO_BINARY_FIRST=$(on-target sha256sum build/galaxy42/tunserver.elf | head -n 1)
	echo "Build with Gitian ($INFO_GITIAN) of $TARGET_NAME $INFO_TARGET gives $INFO_BINARY_FIRST"
}

function all_including_new_lxc() {
	set_env
	gitian_builder_download
	gitian_builder_make_lxc_image
	target_download
	gitian_run_build
	gitian_show_result
}

function all_excluding_new_lxc() {
	set_env
	target_download
	gitian_run_build
	gitian_show_result
}

echo
echo "Usage: call this program with argument 'fast' (faster gbuild, no VM) or 'image' (lxc image rebuild, must be used 1st time)"
echo

if [[ "$1" == "fast" ]] ; then all_excluding_new_lxc ; fi
if [[ "$1" == "image" ]] ; then all_including_new_lxc ; fi




