#!/bin/bash
sha256sum $( find  /home/ubuntu/build/boost/mingw_build/lib/*   /home/ubuntu/build/libsodium/*/bin/   -name  '*.dll' -o -name '*.exe' ; ls -1 /home/ubuntu/build/galaxy42/*exe | sort )
