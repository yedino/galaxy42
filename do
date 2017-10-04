#!/usr/bin/env bash

#
# Does the automated build, should work on Linux, Mac, Windows Cygwin
#

set -o errexit
set -o nounset

function usage_mini {
	echo ""
	echo "--- Short help ---"
	echo ""
	echo "You can set this env options before running this program, example to enable COVERAGE and set EXTLEVEL to preview level"
	echo "for that you could run FOR EXAMPLE:"
	echo ""
	echo "  COVERAGE=0 EXTLEVEL=0  ./do --go  # <--- this is the normal command that builds recommended version"
	echo "  COVERAGE=0 EXTLEVEL=30 ./do --go  # no coverage, build with experimental features enabled (dangerous!!)"
	echo "  COVERAGE=1 EXTLEVEL=0  ./do --go  # coverage - for build test bots"
	echo ""
	echo "For faster multithread build you can use THREADS env variable:"
	echo "  THREADS=2 COVERAGE=1 EXTLEVEL=0  ./do --go"
	echo ""
	echo "Run do --help to see all options, there are special work-arounds."
}

function usage_main {
	echo "========== Usage main ==========="
	echo "ENV OPTIONS that you can set before running the program:"
	echo ""
	echo "COVERAGE=... enables/disables the coverage (for e.g. coveralls test). 0=disable 1=enable."
	echo ""
	echo "EXTLEVEL=... sets level of extension:"
	echo "  0 is the stable code, recommended."
	echo "  10 is the preview code, not recommended yet, for beta-testers at most."
	echo "  20 is the EXPERIMENTAL code, not recommended, for developers only."
	echo "  30 is the EXPERIMENTAL-DANGEROUS code, that we expect to very likely have exploits "
	echo "    or grave bugs, run only in VM for some testets."
	echo ""
	echo "THREADS=... sets threads number that you want to use for faster build. (default THREADS=1) "
	echo ""
	echo "Special options include:"
	echo "BUILD_STATIC=1 if this is set to 1, then we will tell CMake to build static version of the program (it requires static deps)"
	echo "USE_BOOST_MULTIPRECISION_DEFAULT=1 set it to 0 instead to work around broken lib boost multiprecision on e.g. Suse"
	echo ""
	echo "Program command line options:"
	echo "  --help shows the help and exits"
}

function prepare_languages() {
	source "share/script/need_translations.sh"
}

function usage {
	usage_mini
	usage_main
}

function platform_recognize {
	uname -a # show info
	if [[ -n $(uname -a | grep "GNU/Linux") ]]
	then
		platform="gnu_linux"
	elif [[ -n $(uname -a | grep "Cygwin") ]]
	then
		if [[ -n $(uname -a | grep "i686") ]]
		then
			platform="cygwin32"
		elif [[ -n $(uname -a | grep "x86_64") ]]
		then
			platform="cygwin64"
		else
			platform="unknown"
		fi
	elif [[ -n $(uname -a | grep "Darwin") ]]
	then
		platform="mac_osx"
		# readlink on OSX have different behavior than in GNU
		# to get same behavior we could use greadlink from coreutils package
		# brew install coreutils
		shopt -s expand_aliases
		alias readlink="greadlink"
	else
		platform="unknown"
	fi
}

function thread_setting {
	if [[ -z ${THREADS+x} ]]
	then
		echo "THREADS variable is not set, will use default (THREADS=1)"
		readonly THREADS=1
	else
		echo "Will run build script with THREADS=${THREADS}"
	fi
}


function clean_previous_build {
	make clean || { echo "(can not make clean - but this is probably normal at first run)" ; }
	rm -rf CMakeCache.txt CMakeFiles/ || { echo "(can not remove cmake cache - but this is probably normal at first run)" ; }
}

echo ""
echo "------------------------------------------"
echo "The 'do' script - that builds this project"
echo ""

prepare_languages

platform_recognize
echo "Recognized platform: $platform"
readonly dir_base_of_source="$(readlink -e ./)"

thread_setting
# import fail function
. "${dir_base_of_source}"/share/script/lib/fail.sh

clean_previous_build

# download external dependencies/submodules
./download.sh || fail "Downloads failed"

for dir in depends/* ; do
	count=$(find "$dir" | wc -l) ;
	if ((count<2)) ; then
		echo "Error: this submodule seems empty - I see just $count file(s)) - fix it: $dir"
		exit 1
	fi
done

if [[ "${1:-""}" == "-h" ]] ; then
	usage
	exit 2 # <--- exit
fi
if [[ "${1:-""}" == "--help" ]] ; then
	usage
	exit 2 # <--- exit
fi

if [[ "$platform" == "cygwin32" ]]
then

	echo "PLATFORM - WINDOWS/CYGWIN 32-bit ($platform)"

	cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_cygwin_32bit.cmake.in . || fail "Can not cmake (on Cygwin mode)"
	make -j"${THREADS}" tunserver.elf || fail "Can not make (on Cygwin mode)"

	exit 0

elif [[ "$platform" == "cygwin64" ]]
then

	echo "PLATFORM - WINDOWS/CYGWIN 64-bit ($platform)"

	cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_cygwin_64bit.cmake.in . || fail "Can not cmake (on Cygwin mode)"
	make -j"${THREADS}" tunserver.elf || fail "Can not make (on Cygwin mode)"

	exit 0

elif [[ "$platform" == "mac_osx" ]]
then
	echo "PLATFORM - DARWIN ($platform)"
elif [[ "$platform" == "unknown" ]]
then
	fail "Unknown build platform! $(uname -a), abort/fail"
elif [[ "$platform" == "gnu_linux" ]]
then
	echo "PLATFORM - NORMAL POSIX e.g. GNU/Linux ($platform)"
fi

# Checking if variables are unset, if so setting default values
[[ -z "${COVERAGE+x}" ]] && COVERAGE="0"
[[ -z "${EXTLEVEL+x}" ]] && EXTLEVEL="0"
[[ -z "${USE_BOOST_MULTIPRECISION_DEFAULT+x}" ]] && USE_BOOST_MULTIPRECISION_DEFAULT="1"

echo ""
echo "Running currently as:"
echo "* COVERAGE=$COVERAGE"
echo "* EXTLEVEL=$EXTLEVEL"
echo ""
echo "* THREADS=$THREADS"
echo ""
echo "* USE_BOOST_MULTIPRECISION_DEFAULT=$USE_BOOST_MULTIPRECISION_DEFAULT"
echo ""

echo ""
echo "===================================================================="
echo "===================================================================="
echo "===================================================================="
echo ""


COVERAGE="$COVERAGE" EXTLEVEL="$EXTLEVEL" ./build-extra-libs.sh || fail "Building extra libraries failed"

[ -r "toplevel" ] || fail "Run this while being in the top-level directory; Can't find 'toplevel' in PWD=$PWD"
if [[ $OSTYPE == "linux-gnu" ]]; then
	source gettext.sh || fail "Gettext is not installed, please install it."

	lib='utils.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
		eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }

	init_platforminfo || { printf "%s\n" "$(gettext "error_init_platforminfo")" ; exit 1; }
	if (( ! platforminfo[family_detected] )) ; then printf "%s\n" "$(gettext "error_init_platforminfo_unknown")" ; exit 1 ; fi

	# setting newer CC CXX for older ubuntu
	if [[ "${platforminfo[distro]}" == "ubuntu" ]]; then
		# get ubuntu main version e.g. "14" from "ubuntu_14.04"
		ubuntu_ver=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
		# if ubuntu main version is older/equal than 14
		if (( $ubuntu_ver <= 14 )); then
			echo "Setting manually newer compiler for ubuntu <= 14"
			echo "Which gcc-5, g++-5: "
			w_gcc=$(which gcc-5) || true
			w_gpp=$(which g++-5) || true
			if [[ -z "$w_gcc" ]] || [[ -z "$w_gpp" ]]; then
				echo "Can't found g++/gcc in version 5. Aborting"
				echo "Required dependencies can be installed using install.sh script"
				exit 1
			fi
			export CC=gcc-5
			export CXX=g++-5
		fi
	fi

fi

echo "=== language / translations - will compile langauges ==="
contrib/tools/galaxy42-lang-update-all || fail "Compiling po to mo (gettext/translations)"

echo ""
echo "===================================================================="
echo "===================================================================="
echo "===================================================================="
echo ""

echo "Will run cmake, PWD=$PWD USER=$USER, PATH=$PATH"
echo "CC=${CC:-"unset"}, CXX=${CXX:-"unset"}, CPP=${CPP:-"unset"}"
echo "Which gcc, g++: "
which gcc
which g++

FLAG_STATIC="OFF"
if [[ "${BUILD_STATIC:-}"  == "1" ]] ; then
	printf "\n\n\nSTATIC BUILD ENABLED\n\n\n"
	FLAG_STATIC="ON"
fi

set -x
dir_build="$dir_base_of_source/build"
echo "Will build into directory dir_build=$dir_build"
mkdir -p $dir_build
pushd $dir_build
	cmake  ..  \
		-DBUILD_STATIC_TUNSERVER="$FLAG_STATIC" \
		-DEXTLEVEL="$EXTLEVEL" \
		-DCOVERAGE="$COVERAGE" \
		${BUILD_SET_BOOST_ROOT:+"-DBOOST_ROOT=$BUILD_SET_BOOST_ROOT"} \
		${FLAG_BOOST_ROOT:+"$FLAG_BOOST_ROOT"} \
		-DUSE_BOOST_MULTIPRECISION_DEFAULT="$USE_BOOST_MULTIPRECISION_DEFAULT" \
			|| fail "Error: Cmake failed - look above for any other warnings, and read FAQ section in the README.md"
	set +x
	# the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt


	set -x
	ln -s "$dir_base_of_source"/share share || echo "Link already exists"

	make -j"${THREADS}" \
		|| fail "Error: the Make build failed - look above for any other warnings, and read FAQ section in the README.md"

	set +x
popd

