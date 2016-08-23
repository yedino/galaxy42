#!/bin/bash
# This script should be run as-is inside the source code of this project.
# It allows to build&install the programs of this project,
# and also it allows to configure all developer tools.

dir_base_of_source="./" # path to reach the root of source code (from starting location of this script)

source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }
export TEXTDOMAIN="galaxy42_installer"
# share/locale/pl/LC_MESSAGES/galaxy42_installer.mo
export TEXTDOMAINDIR="${dir_base_of_source}share/locale/"
programname="Galaxy42"

source "${dir_base_of_source}/share/script/lib/abdialog.sh" || {\
	echo "Can not find local library abdialog (dir_base_of_source=$dir_base_of_source)" ; exit 1; }

text1="$(eval_gettext "This tool will configure your computer for the SELECTED by you functions of \$programname.")"

text2=""
if (($abdialog_curses)) ; then
text2="$(gettext "This seems to be a text-mode GUI, you can use up/down arrows, SPACE to select option, ENTER to finish.")"
fi

text="\n${text1}\n\n${text2}"

abdialog --title "$(eval_gettext "Configure computer for \$programname")" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--yesno "$text" 20 60 || abdialog_exit

response=$( abdialog  --checklist  "$(gettext "How do you want to use \$programname:")"  23 76 18  \
	"build"         "$(gettext "Build this program from source-code")" "on" \
	"runit"         "$(gettext "Use this program on this computer")" "on" \
	"devel"         "$(gettext "Develop this program (simple)")" "off" \
	"devel2"        "$(gettext "Develop this program (advanced)")" "off" \
	"bgitian"       "$(gettext "Compile and publish (e.g. Gitian)")" "off" \
	2>&1 >/dev/tty || abdialog_exit )

read -r -a tab <<< "$response" ; for item in "${tab[@]}" ; do

function install_for_build() {
	sudo aptitude install git gcc g++ cmake autoconf libtool build-essential \
		libboost-system libboost-filesystem libboost-program-options
}

function install_for_runit() {
	install_for_build
}

function install_for_devel() {
		install_for_build
		install_for_runit
		sudo aptitude install libboost-system-dev libboost-filesystem-dev libboost-program-options-dev
}

function install_for_devel2() {
		install_for_devel
		# in future also add here things for e.g. simulations
}

function install_build_gitian() {
		install_for_build
		install_for_runit
		install_for_devel
		sudo aptitude install lxc
}

case "$item" in
	build)
		install_for_build
	;;
	runit)
		install_for_runit
	;;
	devel)
		install_for_devel
	;;
	bgitian)
		install_build_gitian
	;;
esac



done

exit 1 # XXX

if false ; then
response=$( abdialog  --checklist  "$(gettext "Install (as root) following functions:")"  23 76 18  \
	"yes"            "$(gettext "Do NOT ask confirmation for unsafe settings, I'm expert.")" "off" \
	"setcap"         "$(gettext "For users: Allows us to create virtual network card.")" "on" \
	"forlxc"         "$(gettext "For builders: setup your sys for LXC (for Gitian reproducible)")" "off" \
	"netpriv"        "$(gettext "For devel: allow to create local test network namespaces.")" "off" \
	2>&1 >/dev/tty || abdialog_exit )
fi

text="$(eval_gettext "Finished installation of \$programname.")"
abdialog --title "$(gettext 'Done')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--msgbox "$text" 20 60 || abdialog_exit

