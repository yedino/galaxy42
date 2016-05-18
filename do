#!/bin/bash -e
./download.sh || { echo "Downloads failed" ; exit 1 ; }
./build-extra-libs.sh || { echo "Building extra libraries failed" ; exit 1 ; }
cmake . || { echo "Cmake failed" ; exit 1 ; }
make || { echo "make failed" ; exit 1 ; }

