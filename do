#!/bin/bash -e

function usage_mini {
	echo "You can set this env options before running this program, example to enable COVERAGE and set EXTLEVEL to preview level"
	echo "for that you could run FOR EXAMPLE:"
	echo "  COVERAGE=1 EXTLEVEL=10 ./do"
	echo "  COVERAGE=0 EXTLEVEL=30 ./do --go  # no coverage, build with experimental features, run withot waiting for confirmation"
	echo ""
	echo "Run do --help to see all options"
}

function usage_main {
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
echo ""
echo "Running currently as:"
echo "* COVERAGE=$COVERAGE"
echo "* EXTLEVEL=$EXTLEVEL"
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

cmake .  -DEXTLEVEL="$EXTLEVEL" -DCOVERAGE="$COVERAGE" || { echo "Cmake failed" ; exit 1 ; } # the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt

make || { echo "make failed" ; exit 1 ; }

