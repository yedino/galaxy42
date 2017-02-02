#!/usr/bin/env bash

set -o errexit
set -o nounset

# Required variables:
# readonly BUILD_DIR
# readonly GALAXY_DIR
# readonly OUTDIR
readonly OUTDIR_NAME="out"
readonly APP_FILES_DIR=${OUTDIR} # in our case APP_FILES_DIR should contain files (binaries/libs) and this is the same dir as OUTDIR
readonly APP_NAME="Tunserver.app"

# importing functions
. "${GALAXY_DIR}"/share/script/lib/fail.sh

# ** building bomutils **
pushd "${GALAXY_DIR}"
       contrib/macdeploy/build-bomutils.sh || fail "Failed to build bomutils"
popd

# ** building xar **
pushd "${GALAXY_DIR}"
       contrib/macdeploy/build-xar.sh || fail "Failed to build xar"
popd



function create_PackageInfo() {
       pushd "${GALAXY_DIR}/dmg-build"

               local git_version=`git describe`
               local number_of_files=`find root | wc -l`
               local install_size=`du -b -s -h -k root | cut -f1` # in KBytes
               cat <<< \
'<pkg-info format-version="2" identifier="Tunserver.pkg" version="'"${git_version}"'" install-location="/" auth="root">
  <payload installKBytes="'"${install_size}"'" numberOfFiles="'"${number_of_files}"'"/>
  <scripts>
  </scripts>
<bundle-version>
    <bundle id="com.Galaxy42.Tunserver" CFBundleIdentifier="com.Galaxy42.Tunserver" path="./Applications/'"${APP_NAME}"'" CFBundleVersion="0.1"/>
</bundle-version>
PackageInfo</pkg-info>'

       popd

}

function create_DistribiutonFile() {
       pushd "${GALAXY_DIR}/dmg-build"

               local git_version=`git describe`
               local number_of_files=`find root | wc -l`
               local install_size=`du -b -s -h -k root | cut -f1` # in KBytes

               cat <<< \
"taller-script minSpecVersion=\"1.000000\" authoringTool=\"com.apple.PackageMaker\" authoringToolVersion=\"3.0.3\" authoringToolBuild=\"174\">
    <title>${APP_NAME}</title>
    <options customize=\"never\" allow-external-scripts=\"no\"/>
    <domains enable_anywhere=\"true\"/>
    <installation-check script=\"pm_install_check();\"/>
    <script>function pm_install_check() {
  if(!(system.compareVersions(system.version.ProductVersion,'10.7') >= 0)) {
    my.result.title = 'Failure';
    my.result.message = 'You need at least Mac OS X 10.7 to install Galxa42/Tunserver.';
    my.result.type = 'Fatal';
    return false;
  }
  return true;
}
</script>
    <readme file=\"ReadMe\"/>
    <license file=\"License\"/>
    <choices-outline>
        <line choice=\"choice1\"/>
    </choices-outline>
    <choice id=\"choice1\" title=\"base\">
        <pkg-ref id=\"com.Galaxy42.Tunserver.base.pkg\"/>
    </choice>
    <pkg-ref id=\"com.Galaxy42.Tunservr.base.pkg\" installKBytes=\"${install_size}\" version=\"${number_of_files}\" auth=\"Root\">#base.pkg</pkg-ref>
</installer-script>"

       popd
}

function create_img() {
       #create img
       pushd "${GALAXY_DIR}/dmg-build"
               mkdir /tmp/disk
               cp Tunserver_Installer.pkg /tmp/disk/
               mkisofs -o Tunserver.img  -r -l -ldots -V "Tunserver" /tmp/disk
       popd
}
function create_dmg() {
       pushd "${GALAXY_DIR}/dmg-build"
               dd if=/dev/zero of=/tmp/tunserver.dmg bs=1M count=64
               mkfs.hfsplus -v Tunserver_Installer.pkg /tmp/tunserver.dmg
               cp /tmp/tunserver.dmg "${OUTDIR}"/Tunserver.dmg
       popd
}

function clean_builds() {
       pushd ${OUTDIR}
               rm -rf locale
               rm tunserver.elf
               rm *.dylib
       popd
       rm -rf "${GALAXY_DIR}/dmg-build"
}
function pack_to_dmg() {
       mkdir -p "${GALAXY_DIR}/dmg-build"

       pushd "${GALAXY_DIR}/dmg-build"

               mkdir -p flat/base.pkg flat/Resources/en.lproj
               mkdir -p root/Applications;
               cp -r "${APP_FILES_DIR}" root/Applications/

               mv "root/Applications/${OUTDIR_NAME}" "root/Applications/${APP_NAME}"

               ( cd root && find . | cpio -o --format odc --owner 0:80 | gzip -c ) > flat/base.pkg/Payload

               create_PackageInfo > "flat/base.pkg/PackageInfo"

               "${BUILD_DIR}/bomutils/build/bin/mkbom" -u 0 -g 80 root flat/base.pkg/Bom

               # copy licence and readme
               cp ${GALAXY_DIR}/README.md flat/Resources/ReadMe
               cp ${GALAXY_DIR}/LICENCE.txt flat/Resources/Licence

               create_DistribiutonFile > flat/Distribution

               # create pkg
               ( cd flat && /home/ubuntu/build/xar/xar/src/xar --compression none -cf "../Tunserver_Installer.pkg" * )

               # create dmg
               create_dmg

       popd

       clean_builds
}
pack_to_dmg
