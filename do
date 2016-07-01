#!/bin/bash -e

echo ""
echo "------------------------------------------"
echo "The 'do' script - that builds this project"
[[ -z "$COVERAGE" ]] && COVERAGE="0"
[[ -z "$EXTLEVEL" ]] && EXTLEVEL="0"
echo "COVERAGE=$COVERAGE"
echo "EXTLEVEL=$EXTLEVEL"
echo "You can set this env options before running this program, example to enable COVERAGE and set EXTLEVEL to preview level"
echo "for that you could run:"
echo "COVERAGE=1 EXTLEVEL=10 ./do"
echo ""

echo "===================================================================="
echo "===================================================================="
echo "===================================================================="

make clean || { echo "(can not make clean - but this is probably normal at first run)" ; }
rm -rf CMakeCache.txt CMakeFiles/ || { echo "(can not remove cmake cache - but this is probably normal at first run)" ; }

./download.sh || { echo "Downloads failed" ; exit 1 ; }

COVERAGE="$COVERAGE" EXTLEVEL="$EXTLEVEL" ./build-extra-libs.sh || { echo "Building extra libraries failed" ; exit 1 ; }

cmake .  -DEXTLEVEL="$EXTLEVEL" -DCOVERAGE="$COVERAGE" || { echo "Cmake failed" ; exit 1 ; } # the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt

make || { echo "make failed" ; exit 1 ; }

