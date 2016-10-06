#!/bin/bash
# This script should be run as-is inside the source code of this project.
# It allows to build&install the programs of this project,
# and also it allows to configure all developer tools.
#
# (C) 2016 Yedino team, on BSD 2-clause licence and also licences on same licence as Galaxy main project (you can pick)
#

[ -r "toplevel" ] || { echo "Run this while being in the top-level directory; Can't find 'toplevel' in PWD=$PWD"; exit 1; }
dir_base_of_source="./" # path to reach the root of source code (from starting location of this script)

source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }
export TEXTDOMAIN="galaxy42_installer"
# share/locale/pl/LC_MESSAGES/galaxy42_installer.mo
export TEXTDOMAINDIR="${dir_base_of_source}share/locale/"


text="${text}\n\n$(eval_gettext "L_what_now_about_compiled")"

echo -e "\n\n$text\n\n"

