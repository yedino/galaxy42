#!/usr/bin/env bash

set -o errexit
set -o nounset

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly PACKAGE_NAME="galaxy42"
readonly PKG_PATH="$SCRIPT_DIR/$PACKAGE_NAME.pkg"

readonly GALAXY42_VERSION="$(git describe)"


is_dir() {
    local dir=$1

    [[ -d $dir ]]
}
is_file() {
    local file=$1

    [[ -f $file ]]
}

clean_dirs() {
	echo "cleaning dirs"
	pushd $SCRIPT_DIR
		dir_list=("$@")
		for dir in "${dir_list[@]}"; do
			is_dir $dir	\
				&& rm -rf $dir
		done
	popd
}
clean_files() {
	echo "cleaning files"
	pushd $SCRIPT_DIR
		file_list=("$@")
		for file in "${file_list[@]}"; do
			is_file $file	\
				&& rm $file
		done
	popd
}

# parse file path to get filename, where '/' is delimiter : /usr/bin/gcc -> gcc
get_last_column () {
	eval $1=$( echo "$2" \
		| rev \
		| cut -d/ -f1 \
		| rev )
}

change_tun_dylib_loadpath () {
	local tun_path="$SCRIPT_DIR/tunserver.app/tunserver.elf"

	# dynamic libs that are required by galaxy42
	local dylib_list=( \
		"/usr/local/opt/boost/lib/libboost_locale-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_system-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_filesystem-mt.dylib" \
		"/usr/local/opt/boost/lib/libboost_program_options-mt.dylib" \
		"/usr/local/opt/libsodium/lib/libsodium.18.dylib" )

	# check that required files exist
	! is_file $tun_path \
		&& echo "$tun_path not exist, something went wrong, exiting ..." && exit 1
	
	for dylib in "${dylib_list[@]}"; do
		! is_file $dylib \
			&& echo "missing library: $dylib. This library is required on your machine to create pkg, exiting ..." && exit 1
	done

	for dylib in "${dylib_list[@]}"; do
		last_column='overwrite_me'
		get_last_column last_column $dylib
		install_name_tool -change $dylib "/Applications/Galaxy42/$last_column" $tun_path
	done

}

create_tun_component () {
	echo "creating tun.pkg component"
	local tun_identifier="tunserver"
    local tun_componenet_app="Tunserver.app"
	local tun_plist="tunserver.plist"
	local tun_bin="tunserver.elf"
	local tun_pkg="tunserver.pkg"

	
	pushd $SCRIPT_DIR
		clean_dirs "$tun_componenet_app"

		mkdir $tun_componenet_app

		cp -n "../../$tun_bin" $tun_componenet_app
		change_tun_dylib_loadpath
		
		pkgbuild --analyze --root "$tun_componenet_app" $tun_plist
		pkgbuild --identifier $tun_identifier \
			--root $tun_componenet_app \
			--component-plist $tun_plist \
                        --install-location "/Applications/Galaxy42" \
			$tun_pkg
		
                clean_dirs "$tun_componenet_app"
                clean_files "$tun_plist"
    popd
}
create_boost_component () {
	echo "creating boost.pkg component"
	local boost_identifier="boost_libs"
	local boost_componenet_app="Boost.app"
    local boost_plist="boost.plist"
	local boost_pkg="boost.pkg"
    
	pushd $SCRIPT_DIR
		clean_dirs "$boost_componenet_app"

		mkdir $boost_componenet_app
		cp -n "/usr/local/opt/boost/lib/libboost_locale-mt.dylib" $boost_componenet_app
		cp -n "/usr/local/opt/boost/lib/libboost_system-mt.dylib" $boost_componenet_app
		cp -n "/usr/local/opt/boost/lib/libboost_filesystem-mt.dylib" $boost_componenet_app
		cp -n "/usr/local/opt/boost/lib/libboost_program_options-mt.dylib" $boost_componenet_app

        pkgbuild --analyze --root "$boost_componenet_app/" $boost_plist
		pkgbuild --identifier $boost_identifier \
			--root $boost_componenet_app \
                        --install-location "/Applications/Galaxy42" \
			$boost_pkg
		
		clean_dirs "$boost_componenet_app"
    popd
}
create_sodium_component () {
	echo "creating sodium.pkg component"
	local sodium_identifier="libsodium"
	local sodium_componenet_app="Sodium.app"
    local sodium_plist="sodium.plist"
	local sodium_pkg="sodium.pkg"
    
	pushd $SCRIPT_DIR
		clean_dirs "$sodium_componenet_app"

		mkdir $sodium_componenet_app
		cp -n "/usr/local/opt/libsodium/lib/libsodium.18.dylib" $sodium_componenet_app

        pkgbuild --analyze --root "$sodium_componenet_app" $sodium_plist
		pkgbuild --identifier $sodium_identifier \
			--root $sodium_componenet_app \
                        --install-location "/Applications/Galaxy42" \
			$sodium_pkg
		
		clean_dirs "$sodium_componenet_app"
    popd
}
create_galaxy_pkg() {
	echo "creating galaxy42.pkg"
	create_tun_component
	create_boost_component
	create_sodium_component
    
    pushd $SCRIPT_DIR
		productbuild --synthesize \
				--package tunserver.pkg \
				--package boost.pkg \
				--package sodium.pkg \
				Distribution.xml
		productbuild --distribution ./Distribution.xml \
				--package-path . \
				./Galaxy42_Installer.pkg
    popd
}

main() {
	echo "starting main"
	create_galaxy_pkg
}
main
