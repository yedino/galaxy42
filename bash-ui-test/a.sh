#!/bin/bash
source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }
export TEXTDOMAIN="galaxy42_installer"
# ./localedir/pl/LC_MESSAGES/galaxy42_installer.mo
export TEXTDOMAINDIR="./localedir/"
programname="Galaxy42"

source abdialog.sh

text1="$(gettext "This tool will configure your computer
for the SELECTED by you functions of $programname.
")"

text2=""
if (($abdialog_curses)) ; then
text2="$(gettext "

This seems to be a text-mode GUI,
you can use up/down arrows, SPACE to select option, ENTER to finish.
")"
fi

text="${text1}${text2}"

abdialog --title "$(eval_gettext 'Configure computer for $programname')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--yesno "$text" 20 60 || abdialog_exit

abdialog  --checklist  "$(gettext "Install (as root) following functions:")"  23 78 18  \
	"setcap"         "$("For users: Allows us to create virtual network card.")" "on" \
	"yes"            "$("Do NOT ask confirmation for unsafe settings, I'm expert.")" "off" \
	"forlxc"         "$("For builders: setup your sys for LXC (Gitian reproducible)")" "off" \
	"netpriv"        "$("For devel: allow to create local test network namespaces.")" "off" \
	|| abdialog_exit

