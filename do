#!/bin/bash -e
./download.sh
./build-extra-libs.sh
cmake .
make
