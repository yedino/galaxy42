#!/usr/bin/env bash

set -o errexit
set -o nounset

# sort normalize
export LC_ALL=C

# Required variables:
# readonly BUILD_DIR
# readonly GALAXY_DIR
# readonly OUTDIR
# readonly REFERENCE_DATETIME
# readonly WRAP_DIR
readonly OUTDIR_NAME="out"
readonly APP_FILES_DIR=${OUTDIR} # in our case APP_FILES_DIR should contain files (binaries/libs) and this is the same dir as OUTDIR
readonly APP_NAME="Tunserver.app"

# importing functions
. "${GALAXY_DIR}"/share/script/lib/fail.sh
. "${GALAXY_DIR}"/share/script/lib/create_faketime_wrappers.sh

# setting compilers
export CC=/home/ubuntu/wrapped/gcc
export CXX=/home/ubuntu/wrapped/g++

# ** building bomutils **
pushd "${GALAXY_DIR}"
	. contrib/macdeploy/build-bomutils.sh || fail "Failed to build bomutils"
popd

# ** building xar **
pushd "${GALAXY_DIR}"
	. contrib/macdeploy/build-xar.sh || fail "Failed to build xar"
popd

# wrap mkbom and xar and place them in PATH
export PATH_orig="${PATH}"
export PATH="${PATH}:${BUILD_DIR}/bomutils/build/bin/:${BUILD_DIR}/xar/xar/src/"
create_global_faketime_wrappers "${REFERENCE_DATETIME}" "mkbom xar" "${WRAP_DIR}"
export PATH=${WRAP_DIR}:${PATH}

# unset compilers
unset CC
unset CXX

function create_PackageInfo() {
	pushd "${GALAXY_DIR}/dmg-build" > /dev/null

		local git_version=`git describe`
		local number_of_files=`find root | wc -l`
		local install_size=`du -b -s -h -k root | cut -f1` # in KBytes
		cat <<< \
'<pkg-info format-version="2" identifier="Tunserver_Installer.pkg" version="'"${git_version}"'" install-location="/" auth="root">
  <payload installKBytes="'"${install_size}"'" numberOfFiles="'"${number_of_files}"'"/>
  <scripts>
  </scripts>
<bundle-version>
    <bundle id="com.galaxy42.tunserver" CFBundleIdentifier="com.galaxy42.tunserver" path="./Applications/'"${APP_NAME}"'" CFBundleVersion="1.3.0"/>
</bundle-version>
</pkg-info>'

	popd > /dev/null

}

# function getting one parametr with path to Distribiution file
function create_DistribiutonFile() {
	local FILENAME=$1
	pushd "${GALAXY_DIR}/dmg-build" > /dev/null

		local git_version=`git describe`
		local number_of_files=`find root | wc -l`
		local install_size=`du -b -s -h -k root | cut -f1` # in KBytes

		cat > "${FILENAME}" << EOL
<?xml version="1.0" encoding="utf-8"?>
<installer-script minSpecVersion="1.000000" authoringTool="com.apple.PackageMaker" authoringToolVersion="3.0.3" authoringToolBuild="174">
    <title>${APP_NAME}</title>
    <options customize="never" allow-external-scripts="no"/>
    <domains enable_anywhere="true"/>
    <installation-check script="pm_install_check();"/>
    <script>function pm_install_check() {
  if(!(system.compareVersions(system.version.ProductVersion,'10.5') >= 0)) {
    my.result.title = 'Failure';
    my.result.message = 'You need at least Mac OS X 10.5 to install Galaxy42/Tunserver';
    my.result.type = 'Fatal';
    return false;
  }
  return true;
}
</script>
    <readme file="ReadMe"/>
    <license file="License"/>
    <choices-outline>
        <line choice="choice1"/>
    </choices-outline>
    <choice id="choice1" title="base">
        <pkg-ref id="com.galaxy42.tunserver"/>
    </choice>
    <pkg-ref id="com.galaxy42.tunserver" installKBytes="${install_size}" version="${git_version}" auth="Root">#base.pkg</pkg-ref>
</installer-script>
EOL
	popd
}

function create_dmg() {
	#create img
	pushd "${GALAXY_DIR}/dmg-build"
		mkdir /tmp/disk
		cp Tunserver_Installer.pkg /tmp/disk/
		mkisofs -o Tunserver.dmg  -r -l -ldots -V "Tunserver" /tmp/disk

		cp Tunserver.dmg "${OUTDIR}"/Tunserver.dmg
	popd
}

function clean_builds() {
	pushd ${OUTDIR}
		rm -rf locale
		rm tunserver.elf
		rm *.dylib
	popd

	rm -rf /tmp/disk
	rm -rf "${GALAXY_DIR}/dmg-build"
}
function pack_to_dmg() {

	echo "PATH_dmgbuild=${PATH}}"

	mkdir -p "${GALAXY_DIR}/dmg-build"

	pushd "${GALAXY_DIR}/dmg-build"

		mkdir -p flat/base.pkg flat/Resources/en.lproj
		mkdir -p root/Applications;
		cp -r "${APP_FILES_DIR}" root/Applications/

		mv "root/Applications/${OUTDIR_NAME}" "root/Applications/${APP_NAME}"

		( cd root && find . | sort | tar --no-recursion --mode='u+rw,go+r-w,a+X' --owner=0 --group=0 -c -T - | gzip -c ) > flat/base.pkg/Payload

		create_PackageInfo > "flat/base.pkg/PackageInfo"

		mkbom -u 0 -g 80 root flat/base.pkg/Bom

		# copy licence and readme
		cp ${GALAXY_DIR}/README.md flat/Resources/en.lproj/ReadMe
		cp ${GALAXY_DIR}/LICENCE.txt flat/Resources/en.lproj/Licence

		create_DistribiutonFile "flat/Distribution"

		# create pkg
		( cd flat && xar --compression none -cf "../Tunserver_Installer.pkg" `find | sort` )

		# create dmg
		create_dmg

	popd

	#clean_builds
}
pack_to_dmg

# export original PATH
export PATH="${PATH_orig}"
