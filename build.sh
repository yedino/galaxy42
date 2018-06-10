#!/bin/sh

git submodule update --init --recursive

export CC=/usr/local/bin/egcc
export CXX=/usr/local/bin/eg++
export BOOST_INCLUDEDIR=$HOME/boost/include
export BOOST_LIBRARYDIR=$HOME/boost/lib

cmake .
