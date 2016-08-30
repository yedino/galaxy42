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
	lib='abdialog'; eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }
source "${dir_base_of_source}/share/script/lib/platforminfo.sh" || {\
	lib='platforminfo'; eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }

text1="$(eval_gettext "This tool will configure your computer for the SELECTED by you functions of \$programname.")"

text2=""
if (($abdialog_curses)) ; then
text2="$(gettext "This seems to be a text-mode GUI, you can use up/down arrows, SPACE to select option, ENTER to finish.")"
fi

text3="$(gettext "We recognize your system/platform as:")"
info="$(platforminfo_show_summary)"
text="\n${text1}\n\n${text2}\n\n${text3}\n\n${info}"

abdialog --title "$(eval_gettext "Configure computer for \$programname")" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--yesno "$text" 20 60 2>&1 >/dev/tty || abdialog_exit

response=$( abdialog  --checklist  "$(eval_gettext "How do you want to use \$programname:")"  23 76 18  \
	"warn"          "$(gettext "menu_task_warn")" "off" \
	"build"         "$(gettext "menu_task_build")" "on" \
	"touse"         "$(gettext "menu_task_touse")" "on" \
	"devel"         "$(gettext "menu_task_devel")" "off" \
	"bgitian"       "$(gettext "menu_task_bgitian")" "off" \
	2>&1 >/dev/tty || abdialog_exit )


function install_packets() {
	sudo aptitude install "$@"
}

function install_for_build() {
	install_packets git gcc g++ cmake autoconf libtool build-essential \
		libboost-system-dev libboost-filesystem-dev libboost-program-options-dev
}

function install_for_runit() {
	install_for_build
}

function install_for_devel() {
		install_for_build
		install_for_runit
		install_packets git gpg
}

function install_for_devel2() {
		install_for_devel
		# in future also add here things for e.g. simulations
}

function install_build_gitian() {
		install_for_build
		install_for_runit
		install_for_devel
		install_packets lxc
}

response_menu_task="$response"
warnings_text="" # more warnings
warn_root=0 # things as root
warn_fw=0 # you should use a firewall
warn2_net=1 # warning: strange network settings
enabled_warn=0 # are warnings enabled

read -r -a tab <<< "$response_menu_task" ; for item in "${tab[@]}" ; do
	case "$item" in
		warn)
			enabled_warn=1
		;;
		build)
			warnings_text="${warnings_text}\n(gettext "warning_build")" # install deps
			warn_root=1 # to install deps
		;;
		runit)
			warnings_text="${warnings_text}\n(gettext "warning_runit")" # firewall ipv6, setcap script
			warn_fw=1
			warn_root=1 # for setcap
		;;
		devel)
			warnings_text="${warnings_text}\n(gettext "warning_devel")" # net namespace script
			warn_fw=1 # to test
			warn2_net=1 # namespaces
			warn_root=1 # net namespace, and same as for task runit
		;;
		bgitian)
			warnings_text="${warnings_text}\n(gettext "warning_bgitian")" # run lxc as root, set special NIC card/bridge
			warn2_net=1 # for special LXC network
			warn_root=1 # for LXC and maybe running gitian too (perhaps avoidable?)
		;;
	esac
done

if ((enabled_warn && warn2_net)) ; then
	warnings_text="${warnings_text}\n(gettext "warning_warn2_net")" # net namespace script

	text="$(eval_gettext "warn2_net")"
	abdialog --title "$(gettext 'warn2_net_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--msgbox "$text" 20 60 || abdialog_exit
fi

read -r -a tab <<< "$response_menu_task" ; for item in "${tab[@]}" ; do
	case "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx$item" in
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


text="$(eval_gettext "Finished installation of \$programname.")"
abdialog --title "$(gettext 'Done')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--msgbox "$text" 20 60 || abdialog_exit




