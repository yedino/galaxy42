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

programname="Galaxy42" # shellcheck disable=SC2034

lib='abdialog.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }
lib='utils.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }

# ------------------------------------------------------------------------
# install functions
# ------------------------------------------------------------------------

declare -A done_install # shellcheck disable # that is for shellcheck disable=SC2034
# thought using that disable causes another warning (maybe a bug, debian8)

packages_to_install=() # start with empty list
function install_packages_NOW() { # install selected things
	old=("${packages_to_install[@]}")
	declare -A seen; new=(); for x in "${old[@]}"; do if [[ ! ${seen["$x"]} ]]; then new+=("$x"); seen["$x"]=1; fi; done
	packages_to_install=("${new[@]}")

	printf "\n%s\n" "$(eval_gettext "We will install packages: ${packages_to_install[*]} now (as root)")"
	if (( ${#packages_to_install[@]} > 0 )) ; then
		if (( "verbose" )) ; then
			packages_str="${packages_to_install[*]}"
			text="$(eval_gettext "L_install_packages_text $packages_str")"
			abdialog --title "$(gettext 'install_packages_title')" \
				--yes-button "$(gettext "Install")" --no-button "$(gettext "Quit")" \
				--msgbox "$text" 20 60 || abdialog_exit
		fi

		platforminfo_install_packages "${packages_to_install[@]}" || {
			printf "\n%s\n" "$(eval_gettext "L_install_failed")"
			exit 1
		}
	else
		printf "\n%s\n" "$(eval_gettext "L_install_nothing_to_do")"
	fi
	packages_to_install=() # clear the list, is now installed
}

function install_packages() { # only selects things for install, does not actually do it yet
	packages_to_install+=("$@")
	#echo "Will install more packages: " "${packages_to_install[@]}"
}

# ------------------------------------------------------------------------
# install functions for this project

done_install=()
export done_install # so bashcheck does not complain

# reasons for requiring restart:
needrestart_lxc=0

function install_for_build() {
	(("done_install['install_for_build']")) && return ; done_install['install_for_build']=1
	install_packages git gcc cmake autoconf libtool make automake
	install_packages figlet banner # used in some scripts
	if (("platforminfo[is_family_debian]")) ; then

		if [[ "${platforminfo[distro]}" == "ubuntu" ]]; then
			# get ubuntu main version e.g. "14" from "ubuntu_14.04"
			ubuntu_ver=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
			# if ubuntu main version is older/equal than 14
			if (( ubuntu_ver <= 14 )); then
				run_with_root_privilages "./share/script/setup-ubuntu14-host" || fail
			fi
		fi


		install_packages  g++ build-essential libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libsodium-dev
	elif (("platforminfo[is_family_redhat]")) ; then
		install_packages gcc-c++ boost-devel libsodium-devel
		# EXTLEVEL fftw-devel
	elif (("platforminfo[is_family_alpine]")) ; then
		install_packages g++ libsodium-dev boost-dev make automake # alpine also needs - bash (for scripts!), newt (whiptail)
		# EXTLEVEL fftw-devel
	fi
}

function install_for_touse() {
	(("done_install['install_for_touse']")) && return ; done_install['install_for_touse']=1
	install_for_build
	install_packages sudo
}

function install_for_devel() {
	(("done_install['install_for_devel']")) && return ; done_install['install_for_devel']=1
	install_for_build
	install_for_touse
	install_packages git gnupg
}

function install_for_devel2() {
	(("done_install['install_for_devel2']")) && return ; done_install['install_for_devel2']=1
	install_for_devel
	# in future also add here things for e.g. simulations
}

function install_build_gitian() {
	(("done_install['install_for_build_gitian']")) && return ; done_install['install_for_build_gitian']=1
	install_for_build
	install_for_touse
	install_for_devel

	install_packages lxc apt-cacher-ng debootstrap bridge-utils curl ruby # for Gitian
	install_packages python3-yaml # our scripting aroung Gitian uses this

	install_packages_NOW

	printf "Info: Gitian needs LXC network settings:\n\n"
	if ((is_realstep && verbose2)) ; then show_status "$(gettext "L_now_installing_gitian_lxc")" ; fi
	run_with_root_privilages "./share/script/setup-lxc-host" || fail

	lxc_ourscript="/etc/rc.local.lxcnet-gitian"
	lxc_error=0
	if [[ -r "$lxc_ourscript" ]] ; then
		run_with_root_privilages "bash" "--" "$lxc_ourscript" || lxc_error=1
	else
		lxc_error=1
	fi

	if ((lxc_error)) ; then
		printf "%s\n" "ERROR: Can not run our script $lxc_ourscript - LXC network probably will not work."
	fi

	needrestart_lxc=1
}


# ------------------------------------------------------------------------
# start (main)

sudo_flag="--sudo"
if [[ $EUID -ne 0 ]]; then
	if [[ "$1" == "$sudo_flag" ]]; then
		printf "%s\n" "$(eval_gettext "this_script_uses_sudo_flag \$sudo_flag")"
	else
		printf "%s\n" "$(eval_gettext "this_script_must_be_run_as_root_or \$sudo_flag")"
		exit 1
	fi
fi


init_platforminfo || { printf "%s\n" "$(gettext "error_init_platforminfo")" ; exit 1; }
if (( ! platforminfo[family_detected] )) ; then printf "%s\n" "$(gettext "error_init_platforminfo_unknown")" ; exit 1 ; fi
# platforminfo_install_packages 'vim' 'mc' || { echo "Test install failed." ; exit 1; }  ; echo "Test seems ok." ; exit 0 # debug

text1="$(eval_gettext "This tool will configure your computer for the SELECTED by you functions of \$programname.")"
text_warn="$(gettext "L_program_is_pre_pre_alpha")"

text2=""
if (("$abdialog_curses")) ; then
text2="$(gettext "This seems to be a text-mode GUI, you can use up/down arrows, SPACE to select option, ENTER to finish.")"
fi

text3="$(gettext "We recognize your system/platform as:")"
info="$(platforminfo_show_summary)"
text="\n${text1}\n\n${text_warn}\n\n${text2}\n\n${text3}\n${info}"
# shellcheck disable=SC2069
abdialog --title "$(eval_gettext "Configure computer for \$programname")" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--yesno "$text" 20 60 2>&1 >/dev/tty || abdialog_exit # shellcheck disable=SC2069

# shellcheck disable=SC2069
response=$( abdialog  --menu  "$(eval_gettext "menu_main_title \$programname:")"  23 76 16  \
	"normal"        "$(gettext "menu_taskpack_normal_builduse")"  \
	"custom"        "$(gettext "menu_taskpack_custom")" \
	"x_build_use"   "$(gettext "menu_taskpack_quick_builduse")"  \
	"x_devel"       "$(gettext "menu_taskpack_quick_devel")"  \
	2>&1 >/dev/tty ) || abdialog_exit
[[ -z "$response" ]] && exit


response_menu_task=""

if [[ "$response" == "normal" ]] ; then response_menu_task="warn build touse" ; fi
if [[ "$response" == "x_build_use" ]] ; then response_menu_task="build touse" ; fi
if [[ "$response" == "x_devel" ]] ; then response_menu_task="build touse devel bgitian" ; fi
if [[ "$response" == "custom" ]] ; then
# shellcheck disable=SC2069
response=$( abdialog  --checklist  "$(eval_gettext "How do you want to use \$programname:")"  23 76 18  \
	"warn"          "$(gettext "menu_task_warn")" "on" \
	"build"         "$(gettext "menu_task_build")" "on" \
	"touse"         "$(gettext "menu_task_touse")" "on" \
	"devel"         "$(gettext "menu_task_devel")" "off" \
	"bgitian"       "$(gettext "menu_task_bgitian")" "off" \
	"verbose"       "$(gettext "menu_task_verbose")" "off" \
	2>&1 >/dev/tty ) || abdialog_exit
	response_menu_task="$response"
fi

[[ -z "$response_menu_task" ]] && exit



warnings_text="" # more warnings
warn_ANY=0 # any warning?
warn_root=0 # things as root
warn_fw=0 # you should use a firewall
warn2_net=0 # warning: strange network settings (e.g. lxc br)
enabled_warn=0 # are warnings enabled
verbose=0 # shellcheck disable=SC2034

read -r -a tab <<< "$response_menu_task" ; for item in "${tab[@]}" ; do
	case "$item" in
		warn)
			enabled_warn=1
		;;
		verbose)
			verbose=1 # shellcheck disable=SC2034
		;;

		build)
			warnings_text="${warnings_text}$(gettext "warning_build")\n" # install deps
			warn_root=1 # to install deps
			warn_ANY=1
		;;
		touse)
			warnings_text="${warnings_text}$(eval_gettext "warning_touse \$programname")\n" # firewall ipv6, setcap script
			warn_fw=1
			warn_root=1 # for setcap
			warn_ANY=1
		;;
		devel)
			warnings_text="${warnings_text}$(gettext "warning_devel")\n" # net namespace script
			warn_fw=1 # to test
			warn2_net=1 # namespaces
			warn_root=1 # net namespace, and same as for task touse
			warn_ANY=1
		;;
		bgitian)
			warnings_text="${warnings_text}$(gettext "warning_bgitian")\n" # run lxc as root, set special NIC card/bridge
			warn2_net=1 # for special LXC network
			warn_root=1 # for LXC and maybe running gitian too (perhaps avoidable?)
			warn_ANY=1
		;;
	esac
done

any=0
ww=""
warnings_text="$(gettext "tasks_we_will_do")\n${warnings_text}"
if ((warn_root)) ; then any=1; ww="$(gettext "warn_root")\n${ww}" ; fi
if ((warn_fw)) ; then any=1; ww="$(gettext "warn_fw")\n${ww}" ; fi
if ((any)) ; then warnings_text="${ww}\n${warnings_text}" ; fi

# show basic warnings:
if ((enabled_warn && warn_ANY)) ; then
	text="$(eval_gettext "warning_SUMMARY")\n\n$warnings_text"
	abdialog --title "$(gettext 'warning_SUMMARY_TITLE')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--yesno "$text" 20 60 || abdialog_exit
fi

# show special warnings:
if ((enabled_warn && warn2_net)) ; then
	warnings_text="${warnings_text}\n(gettext 'warning_warn2_net')" # net namespace script

	text="$(eval_gettext "warn2_net")"
	abdialog --title "$(gettext 'warn2_net_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--yesno "$text" 20 60 || abdialog_exit
fi

function show_status() {
	local text="$1"
	abdialog --title "$(gettext 'install_progress_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--msgbox "$text" 20 60 || abdialog_exit
}



read -r -a tab <<< "$response_menu_task" ; for item_tab in "${tab[@]}" ; do
	item="$(echo "$item_tab" | tr -cd '[[:alnum:]]._-' )"
	printf "\n%s\n" "Doing installation task: [$item]"

	nonsteps=(warn verbose)
	is_realstep=1
	inarray "$item" "${nonsteps[@]}" && is_realstep=0

	if ((is_realstep && verbose2)) ; then show_status "$(gettext "status_done_step_BEFORE")${item}" ; fi

	case "$item" in
		build)
			install_for_build
		;;
		touse)
			install_for_touse
		;;
		devel)
			install_for_devel
		;;
		bgitian)
			install_build_gitian
		;;
	esac

	if (("is_realstep" && "verbose")) ; then
		printf "\n\n%s\n%s\n" "$(eval_gettext "status_done_step \$item")" "$(gettext "status_done_step_PRESSKEY")"
		(("verbose2")) && read _
	fi
done

install_packages_NOW

# restart warnings:
any=0
ww=""
if ((needrestart_lxc)) ; then any=1; ww="$(gettext "L_needrestart_LXC_maybe")\n${ww}" ; fi
if ((any)) ; then
	text="$(eval_gettext "L_needrestart_summary_text")\n\n$ww"
	abdialog --title "$(gettext 'L_needrestart_summary_title')" \
		--msgbox "$text" 20 60 || abdialog_exit
fi



text="$(eval_gettext "Finished installation of \$programname.")"
abdialog --title "$(gettext 'Done')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--msgbox "$text" 20 60 || abdialog_exit




