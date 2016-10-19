#!/bin/bash -e

function fail() {
	echo "Error (in $0): " "$@"
	exit 1
}

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
	echo "Special options include:"
	echo "USE_BOOST_MULTIPRECISION_DEFAULT=1 set it to 0 instead to work around broken lib boost multiprecision on e.g. Suse"
	echo ""
	echo "Program command line options:"
	echo "  --help shows the help and exits"
	echo "  --go skips waiting for confirmation from the user"
}

function usage {
	usage_mini
	usage_main
}

echo ""
echo "------------------------------------------"
echo "The 'do' script - that builds this project"
echo ""

if [[ "$1" == "--help" ]] ; then
	usage
	exit 2 # <--- exit
fi

[[ -z "$COVERAGE" ]] && COVERAGE="0"
[[ -z "$EXTLEVEL" ]] && EXTLEVEL="0"

[[ -z "$USE_BOOST_MULTIPRECISION_DEFAULT" ]] && USE_BOOST_MULTIPRECISION_DEFAULT="1"

echo ""
echo "Running currently as:"
echo "* COVERAGE=$COVERAGE"
echo "* EXTLEVEL=$EXTLEVEL"
echo ""
echo "* USE_BOOST_MULTIPRECISION_DEFAULT=$USE_BOOST_MULTIPRECISION_DEFAULT"
echo ""
if [[ "$1" == "--go" ]] ; then
	:
else
	usage_mini
	echo ""
	echo "Press ENTER to continue. Run this script with option --go to skip this pause/help (e.g. in batch mode)"
	read _
fi

echo ""
echo "===================================================================="
echo "===================================================================="
echo "===================================================================="
echo ""


make clean || { echo "(can not make clean - but this is probably normal at first run)" ; }
rm -rf CMakeCache.txt CMakeFiles/ || { echo "(can not remove cmake cache - but this is probably normal at first run)" ; }

./download.sh || { echo "Downloads failed" ; exit 1 ; }

COVERAGE="$COVERAGE" EXTLEVEL="$EXTLEVEL" ./build-extra-libs.sh || { echo "Building extra libraries failed" ; exit 1 ; }

[ -r "toplevel" ] || { echo "Run this while being in the top-level directory; Can't find 'toplevel' in PWD=$PWD"; exit 1; }
dir_base_of_source="./"
if [[ $OSTYPE == "linux-gnu" ]]; then
	source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }

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

contrib/tools/galaxy42-lang-update-all || fail "Compiling po to mo (gettext/translations)"


echo "Will run cmake, PWD=$PWD USER=$USER, CC=$CC, CXX=$CXX, CPP=$CPP, PATH=$PATH"
echo "Which gcc, g++: "
which gcc
which g++

cmake  .  \
	-DEXTLEVEL="$EXTLEVEL" -DCOVERAGE="$COVERAGE" \
	-DUSE_BOOST_MULTIPRECISION_DEFAULT="$USE_BOOST_MULTIPRECISION_DEFAULT" \
	|| { echo "Error: Cmake failed - look above for any other warnings, and read FAQ section in the README.md" ; exit 1 ; }
# the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt

make -j 8 || { echo "Error: the Make build failed - look above for any other warnings, and read FAQ section in the README.md" ; exit 1 ; }

