#!/bin/bash -e
make clean || { echo "(can not make clean - but this is probably normal at first run)" ; }
rm -rf CMakeCache.txt CMakeFiles/ || { echo "(can not remove cmake cache - but this is probably normal at first run)" ; }

./download.sh || { echo "Downloads failed" ; exit 1 ; }

./build-extra-libs.sh || { echo "Building extra libraries failed" ; exit 1 ; }

cmake . || { echo "Cmake failed" ; exit 1 ; } # the build type CMAKE_BUILD_TYPE is as set in CMakeLists.txt
make || { echo "make failed" ; exit 1 ; }

