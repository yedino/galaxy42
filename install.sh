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
	printf "\n%s\n" "$(eval_gettext "Can not find script library \$lib (dir_base_of_source=\$dir_base_of_source).")" ; exit 1; }
lib='utils.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	printf "\n%s\n" "$(eval_gettext "Can not find script library \$lib (dir_base_of_source=\$dir_base_of_source).")" ; exit 1; }
lib='g42-middle-utils.sh' ; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	printf "\n%s\n" "$(eval_gettext "Can not find script library \$lib (dir_base_of_source=\$dir_base_of_source).")" ; exit 1; }

# ------------------------------------------------------------------------

function fail() {
	printf "\n\n\n"
	echo "Error: (in $0) - " "$@"
	printf "\n\n\n"
	infile="$0"
	echo "$(eval_gettext "L_install_fail infile=\$infile") "  ' - ' "$@"
	echo "$(eval_gettext "L_install_fail2")"
	exit 1
}

# ------------------------------------------------------------------------
# install functions
# ------------------------------------------------------------------------

declare -A done_install

warnings_text="" # more warnings
warn_ANY=0 # any warning?
warn_root=0 # things as root
warn_fw=0 # you should use a firewall
warn2_net=0 # warning: strange network settings (e.g. lxc br)
enabled_warn=0 # are warnings enabled
verbose=0 # shellcheck disable=SC2034
autoselect=0 # automatically choose some easy questions

packages_to_install=() # start with empty list
function install_packages_NOW() { # install selected things
	old=("${packages_to_install[@]}")
	declare -A seen; new=(); for x in "${old[@]}"; do if [[ ! ${seen["$x"]} ]]; then new+=("$x"); seen["$x"]=1; fi; done
	packages_to_install=("${new[@]}")
	packages_str="${packages_to_install[*]}"
	export packages_str # so that shellcheck can fuckoff. yeah "shellcheck disable" is not working either
## shellcheck disable=SC2034 (used in eval_gettext)

	printf "\n%s\n" "$(eval_gettext "We will install packages: \$packages_str now (as root)")"
	if (( ${#packages_to_install[@]} > 0 )) ; then
		if (( "verbose" )) ; then
			text="$(eval_gettext "L_install_packages_text \$packages_str")"
			abdialog --title "$(gettext 'install_packages_title')" \
				--yes-button "$(gettext "Install")" --no-button "$(gettext "Quit")" \
				--msgbox "$text" 20 60 || abdialog_exit
		fi

		platforminfo_install_packages "${packages_to_install[@]}" || {
			printf "\n%s\n" "$(gettext "L_install_failed")"
			exit 1
		}
	else
		printf "\n%s\n" "$(gettext "L_install_nothing_to_do")"
	fi
	packages_to_install=() # clear the list, is now installed
}

function install_packages() { # only selects things for install, does not actually do it yet
	packages_to_install+=("$@")
	#echo "Will install more packages: " "${packages_to_install[@]}"
}

function show_status() {
	local text="$1"
	abdialog --title "$(gettext 'install_progress_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--msgbox "$text" 20 50 || abdialog_exit
}

function show_info() {
	local text="$1"
	abdialog --title "$(gettext 'install_info_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--msgbox "$text" 20 65 || abdialog_exit
}

function show_fix() {
	local text="$1"
	abdialog --title "$(gettext 'install_progress_fix')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--msgbox "$text" 21 72 || abdialog_exit
}

# ------------------------------------------------------------------------
# install functions for this project

declare -A done_install

# reasons for requiring restart:
needrestart_lxc=0

function install_for_build() {
	(("done_install['install_for_build']")) && return ; done_install['install_for_build']=1
	install_packages git gcc cmake autoconf libtool make automake
	install_packages figlet # used in some scripts
	if (("platforminfo[is_family_debian]")) ; then

		if [[ "${platforminfo[distro]}" == "ubuntu" ]]; then
			# get ubuntu main version e.g. "14" from "ubuntu_14.04"
			ubuntu_ver=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
			# if ubuntu main version is older/equal than 14
			if (( ubuntu_ver <= 14 )); then
				show_fix "$(gettext 'L_fix_ubuntu_old_build')"
				run_with_root_privilages "./share/script/setup-ubuntu14-host" || fail
			fi
		fi


		install_packages  g++ build-essential libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libsodium-dev gettext
		install_packages libboost-locale-dev
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
	install_packages sudo gettext
}

function install_for_devel() {
	(("done_install['install_for_devel']")) && return ; done_install['install_for_devel']=1
	install_for_build
	install_for_touse
	install_packages git gnupg gettext
}

function install_for_devel2() {
	(("done_install['install_for_devel2']")) && return ; done_install['install_for_devel2']=1
	install_for_devel
	install_packages g++-mingw-w64-i686 g++-mingw-w64-x86-64 # cross build, checking DDLs etc
	# in future also add here things for e.g. simulations
}

function resolv_conf_guess_lxc_usable_lines() {
	cat "$1" | \
		egrep -v -E  '^[[:space:]]*#+.*$' | # skip comment \
		egrep -v -E '^[[:space:]]*$' | # skip empty \
		egrep -v -E -i '^[[:space:]]*nameserver[[:space:]]+127\..*$' | # skip 127 \
		egrep -v -E -i '^[[:space:]]*nameserver[[:space:]]+0\.0\.0\.0.*$' | # skip 0.0.0.0 \
		egrep -v -E -i '^[[:space:]]*nameserver[[:space:]]+localhost*$' | # skip localhost \
		egrep    -E -i '^[[:space:]]*nameserver[[:space:]]+[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}.*$' # SELECT only IPv4
}

function install_build_gitian() {
	(("done_install['install_for_build_gitian']")) && return ; done_install['install_for_build_gitian']=1
	install_for_build
	install_for_touse
	install_for_devel

	install_packages lxc debootstrap bridge-utils curl ruby # for Gitian

	### cgroupfs
	# for systems that are missing proper cgroupfs mounts
	if [[ "${platforminfo[id]}" == "devuan" ]]; then
		show_fix "$(gettext 'L_fix_cgroupfs_mount')"
		install_packages cgroupfs-mount
	fi

	### apt-cache
	printf "\n\n\napt-cacher version selection / install / remove\n"
	midutils_detect_correct_apt_cacher_version || fail "Can not detect apt cacher version"
	# printf "\nXXX midutils_apt_cacher_version_name_service: $midutils_apt_cacher_version_name_service \n\n" ; read _ # XXX

	apt_cacher_bad="$midutils_apt_cacher_version_name_bad"
	apt_cacher_good="$midutils_apt_cacher_version_name_good"
	apt_cacher_service="$midutils_apt_cacher_version_name_service"

	printf "\n\nWill test for bad package%s\n" "${apt_cacher_bad}"

	if platforminfo_checkinstalled_package "$apt_cacher_bad" ; then
		# echo "installed ${apt_cacher_bad}. " ; read _ # debug
		show_fix "$(eval_gettext "L_fix_uninstall_apt_cacher bad=\$apt_cacher_bad good=\$apt_cacher_good.")" \
			|| fail "Confirm to remove bad apt cacher ($apt_cacher_bad)"
		platforminfo_remove_packages "$apt_cacher_bad"
	fi
	install_packages "$apt_cacher_good"

	### other fixes

	install_packages lxc

	install_packages python3-yaml # our scripting aroung Gitian uses this

	install_packages_NOW

	printf "\n\n\nInfo: Gitian needs LXC network settings:\n\n"
	if ((is_realstep && verbose2)) ; then show_status "$(gettext "L_now_installing_gitian_lxc")" ; fi

	### lxc-net: IP forwarding
	local lxc_all_s='--all-if'
	local lxc_cards_s=''

	if (( ! autoselect)) ; then
		show_info "$(gettext 'L_install_option_lxcnet_bridged_INFO')"
		response=$( abdialog  --menu  "$(gettext "L_install_option_lxcnet_bridged_TITLE")"  23 76 16  \
			"all"        "$(gettext "L_install_option_lxcnet_bridged_ITEM_all")"  \
			"some"       "$(gettext "L_install_option_lxcnet_bridged_ITEM_some")"  \
			2>&1 >/dev/tty ) || abdialog_exit
		[[ -z "$response" ]] && exit

		if [[ "$response" == "all" ]] ;  then lxc_all_s="--all-if" ; fi
		if [[ "$response" == "some" ]] ; then
			lxc_all_s="--some-if" ;
			ui_cards=$(abdialog  --inputbox "$(gettext "L_install_option_lxcnet_bridged_INPUTBOX_cards")" \
				20 70 "eth0 eth1 eth2 eth3 usb0 usb1 usb2 wlan0 wlan1 wlan2" \
				2>&1 >/dev/tty ) || abdialog_exit
			lxc_cards_s="$ui_cards"
		fi
	else
		printf "\n\nUsing default lxc-net settings\n\n"
	fi

	run_with_root_privilages "./share/script/setup-lxc-host" '--normal' "$lxc_all_s" "$lxc_cards_s" || fail
	# ^- script install LXC settings, and also should load it right now

	if ((lxc_error)) ; then
		printf "%s\n" "ERROR: Can not run our script $lxc_ourscript - LXC network probably will not work."
	fi

	### lxc-net: DNS access, on host

	dns_fix='resolv_chattr'

	# quick hack to try to count possibly-good (not-localhost) DNS that could work in LXC:
	resolve_file="/etc/resolv.conf"
	dns_lines_good=$( resolv_conf_guess_lxc_usable_lines "$resolve_file" | wc -l )

	if (( dns_lines_good < 1 )) ; then
		show_info "$(gettext "L_install_option_lxcnet_dns_needsfix")" ;
	fi

	if (( (! autoselect) || (dns_lines_good<1) )) ; then
		show_info "$(gettext "L_install_option_lxcnet_dns_INFO")"

		response=$( abdialog  --menu  "$(gettext "L_install_option_lxcnet_dns_TITLE")"  23 76 16  \
			"none" "$(gettext "L_install_option_lxcnet_dns_ITEM_none")"  \
			"resolv_d" "$(gettext "L_install_option_lxcnet_dns_ITEM_resolv_d")"  \
			"resolv_only" "$(gettext "L_install_option_lxcnet_dns_ITEM_resolv_only")"  \
			"resolv_chattr" "$(gettext "L_install_option_lxcnet_dns_ITEM_resolv_chattr")"  \
			2>&1 >/dev/tty ) || abdialog_exit
		dns_fix="$response"
		printf "\n%s\n" "Will use DNS fix: $dns_fix"

		run_with_root_privilages "./share/script/setup-lxc-host-dns" "$dns_fix" || fail "Can not apply selected DNS fix"
	fi

	needrestart_lxc=1
}

function install_captool() {
	install_for_touse
	install_for_devel
	install_packages libcap2-bin  sudo
	install_packages_NOW # we need sudo right now

	echo "--------------------------------------------------------------"

	pwd="$PWD"
	#./share/script/install-as-root/setcap_scripts/install
	cd ./share/script/install-as-root/setcap_scripts/
	run_with_root_privilages "./install" "-y" || fail "Installer of the captool failed."


	echo "--------------------------------------------------------------"
	echo "--------------------------------------------------------------"
	echo "Installing 'captool' :"
	echo
	echo "Now we will open sudo configuration program (visudo)"
	echo
	echo "In that program, copy/paste following line:"
	echo "alice ALL=(ALL)NOPASSWD:/usr/local/bin/setcap_net_admin --normal -u --current -f *"
	echo "... but, replace the first word ('alice') with the user name of the user that should be allowed to build this program and grant it CAP_NET_ADMIN rights"
	echo ""
	echo "Press ENTER when ready to edit the configuration."
	echo ""
	read _
	run_with_root_privilages "visudo" || fail "Program visudo (for captool) failed."

	cd "$pwd"
}

function install_languages() {
	printf "\n%s\n\n" "Install languages."
	contrib/tools/galaxy42-lang-update-all || fail "Update languages"
	printf "\n%s\n" "Install languages - DONE."
}

# ------------------------------------------------------------------------
# start (main)

### resolv_conf_guess_lxc_usable_lines "$1" ; echo "test exit" ; exit 1 ; # XXX


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
	"simple"        "$(gettext "menu_taskpack_normal_builduse")"  \
	"devel"         "$(gettext "menu_taskpack_devel_builduse")"  \
	"devel2"         "$(gettext "menu_taskpack_devel_builduse") (2)"  \
	"x_build_use"   "$(gettext "menu_taskpack_quick_builduse")"  \
	"x_devel"       "$(gettext "menu_taskpack_quick_devel")"  \
	"x_devel2"       "$(gettext "menu_taskpack_quick_devel") (2)"  \
	"custom"        "$(gettext "menu_taskpack_custom")" \
	2>&1 >/dev/tty ) || abdialog_exit
[[ -z "$response" ]] && exit


response_menu_task=""

if [[ "$response" == "simple" ]] ; then response_menu_task="warn build touse verbose" ; fi
if [[ "$response" == "devel" ]] ; then response_menu_task="warn build touse devel bgitian verbose" ; fi
if [[ "$response" == "devel2" ]] ; then response_menu_task="warn build touse devel devel2 bgitian verbose captool" ; fi
if [[ "$response" == "x_build_use" ]] ; then response_menu_task="build touse autoselect" ; fi
if [[ "$response" == "x_devel" ]] ; then response_menu_task="build touse devel bgitian autoselect" ; fi
if [[ "$response" == "x_devel2" ]] ; then response_menu_task="build touse devel devel2 bgitian autoselect captool" ; fi
if [[ "$response" == "custom" ]] ; then
# shellcheck disable=SC2069
response=$( abdialog  --checklist  "$(eval_gettext "How do you want to use \$programname:")"  23 76 18  \
	"build"         "$(gettext "menu_task_build")" "on" \
	"touse"         "$(gettext "menu_task_touse")" "on" \
	"devel"         "$(gettext "menu_task_devel")" "off" \
	"devel2"        "$(gettext "menu_task_devel") 2" "off" \
	"captool"       "$(gettext "menu_task_captool")" "off" \
	"bgitian"       "$(gettext "menu_task_bgitian")" "off" \
	"warn"          "$(gettext "menu_task_warn")" "on" \
	"verbose"       "$(gettext "menu_task_verbose")" "on" \
	"autoselect"    "$(gettext "menu_task_autoselect")" "off" \
	2>&1 >/dev/tty ) || abdialog_exit
	response_menu_task="$response"
fi

[[ -z "$response_menu_task" ]] && exit



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
		devel2)
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
		captool)
			warnings_text="${warnings_text}$(gettext "warning_captool")\n" # captool
			warn_root=1 # for captool
			warn_ANY=1
		;;
		autoselect)
			autoselect=1
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
	text="$(eval_gettext "warn2_net")"
	abdialog --title "$(gettext 'warn2_net_title')" \
		--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
		--yesno "$text" 20 60 || abdialog_exit
fi


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
		devel2)
			install_for_devel2
		;;
		bgitian)
			install_build_gitian
		;;
		captool)
			install_captool
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
	text="$(gettext "L_needrestart_summary_text")\n\n$ww"
	abdialog --title "$(gettext 'L_needrestart_summary_title')" \
		--msgbox "$text" 20 60 || abdialog_exit
fi

install_languages || fail "Installing languages"

text="$(eval_gettext "Finished installation of \$programname.")"
text="${text}\n\n$(eval_gettext "L_what_now")"
abdialog --title "$(gettext 'Done')" \
	--yes-button "$(gettext "Ok")" --no-button "$(gettext "Quit")" \
	--msgbox "$text" 20 60 || abdialog_exit

share/script/show-source-help.sh


