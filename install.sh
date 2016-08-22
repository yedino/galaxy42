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

text1="$(gettext "This tool will configure your computer
for the SELECTED by you functions of $programname.
")"

text2=""
if (($abdialog_curses)) ; then
text2="$(gettext "Hmm... This seems to be a text-mode GUI, you can use up/down arrows, SPACE to select option, ENTER to finish.")"
fi

text="${text1}${text2}"

abdialog --title "$(eval_gettext 'Configure computer for $programname')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--yesno "$text" 20 60 || abdialog_exit

foo=$( abdialog  --checklist  "$(gettext "Install (as root) following functions:")"  23 78 18  \
	"setcap"         "$(gettext "For users: Allows us to create virtual network card.")" "on" \
	"yes"            "$(gettext "Do NOT ask confirmation for unsafe settings, I'm expert.")" "off" \
	"forlxc"         "$(gettext "For builders: setup your sys for LXC (for Gitian reproducible)")" "off" \
	"netpriv"        "$(gettext "For devel: allow to create local test network namespaces.")" "off" \
	2>&1 >/dev/tty || abdialog_exit )

text="$(gettext "Finished installation of $programname.")"
abdialog --title "$(eval_gettext 'Done')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--msgbox "$text" 20 60 || abdialog_exit

