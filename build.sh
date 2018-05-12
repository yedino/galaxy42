#!/bin/sh

git submodule update --init --recursive

export BOOST_INCLUDEDIR=$HOME/boost/include
export BOOST_LIBRARYDIR=$HOME/boost/lib

cmake .
