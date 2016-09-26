#!/bin/bash
cd "${BASH_SOURCE%/*}" || exit # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028
git log --format="%H" -1 || { echo "(error: can not detect gitrev. pwd=$PWD)" ; exit 1; }
