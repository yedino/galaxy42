#!/bin/bash -e

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

cmake  .  \
	-DEXTLEVEL="$EXTLEVEL" -DCOVERAGE="$COVERAGE" \
	-DUSE_BOOST_MULTIPRECISION_DEFAULT="$USE_BOOST_MULTIPRECISION_DEFAULT" \
	|| { echo "Error: Cmake failed - look above for any other warnings, and read FAQ section in the README.md" ; exit 1 ; }
# the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt

make || { echo "Error: the Make build failed - look above for any other warnings, and read FAQ section in the README.md" ; exit 1 ; }

