#!/bin/bash
echo "Entering dir of script $0 (before that, pwd=$PWD)"
cd "${BASH_SOURCE%/*}" || { echo "(error: can not cd into dir)" ; exit 20 ; } # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028

vardir="var/"
rm -rf "$vardir" ; mkdir "$vardir" ; cd "$vardir"
datadir="data/"

echo "Starting test (in PWD=$PWD), expected data is in $datadir, and we will download to $vardir."

echo "TODO"

rm -rf "$vardir"

echo "All done in $0."

