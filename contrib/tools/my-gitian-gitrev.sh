#!/bin/bash
cd "${BASH_SOURCE%/*}" || { echo "(error: can not cd into dir)" ; exit 20 ; } # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028
git log --format="%H" -1 || { echo "(error: can not detect gitrev. pwd=$PWD)" ; exit 1; }
