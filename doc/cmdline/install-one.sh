#!/bin/bash
# (C) 2017 Yedino team, on BSD 2-clause licence and also licences on same licence as Galaxy main project (you can pick)

dir_base_of_source="../../" # path to reach the root of source code (from starting location of this script)

source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }
export TEXTDOMAIN="galaxy42_installer"
# share/locale/pl/LC_MESSAGES/galaxy42_installer.mo
export TEXTDOMAINDIR="${dir_base_of_source}share/locale/"

programname="Galaxy42-developer-menu" # shellcheck disable=SC2034

lib='abdialog.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }
lib='utils.sh'; source "${dir_base_of_source}/share/script/lib/${lib}" || {\
	eval_gettext "Can not find script library $lib (dir_base_of_source=$dir_base_of_source)" ; exit 1; }

dialog_h=$(tput lines) ; dialog_w=$(tput cols)
dialogB_h="$dialog_h" ; dialogB_w="$dialog_w"

dialog_h=$((dialog_h - 2))
dialog_h=$(( dialog_h>30 ? 30 : dialog_h ))
dialog_h2=$((dialog_h - 7 ))
dialog_w=$((dialog_w - 2))
dialog_w=$(( dialog_w>100 ? 100 : dialog_w ))

dialogB_h=$(( dialogB_h>40 ? dialogB_h-5 : dialogB_h ))
dialogB_w=$(( dialogB_w>100 ? dialogB_w-5 : dialogB_w ))
dialogB_h=$(( dialogB_h>300 ? 300 : dialogB_h ))
dialogB_w=$(( dialogB_w>300 ? 300 : dialogB_w ))


# ------------------------------------------------------------------------
# start

init_platforminfo || { printf "%s\n" "$(gettext "error_init_platforminfo")" ; exit 1; }
if (( ! platforminfo[family_detected] )) ; then printf "%s\n" "$(gettext "error_init_platforminfo_unknown")" ; exit 1 ; fi
# platforminfo_install_packages 'vim' 'mc' || { echo "Test install failed." ; exit 1; }  ; echo "Test seems ok." ; exit 0 # debug

xtarget="$HOME/.devel/"
only_file="galaxy42.cmdline"

choices=()
while IFS= read -r line; do
	dir="$line"
	file="$dir/$only_file"
	if [[ -d "$dir" ]] ; then
		if [[ -r "$file" ]] ; then
			echo "Adding dir: [$dir]"
			choices+=("${dir}")
			choices+=("Install ${file}")
		fi
	fi
done < <( ls -1 )

# shellcheck disable=SC2069
response=$( abdialog  --menu  "Install commandline for galaxy 'make run' and for 'tunserver.sh'. File will be copied into $xtarget" \
	"$dialog_h" "$dialog_w" "$dialog_h2"  \
	"${choices[@]}" \
	"(nothing)" "(nothing)" \
	2>&1 >/dev/tty ) || abdialog_exit
[[ -z "$response" ]] && exit
[[ "$response" == "(nothing)" ]] && abdialog_fail "Canceled."
xfrom="$response/$only_file"

mkdir -p "$xtarget" || "Error: can not create dir [$xtarget]"

filedata=$(<"$xfrom")
abdialog_info_quit_big "Will install following default commandline options (one option per line): \n\n\n${filedata}" \
	"preview" "$dialogB_h" "$dialogB_w"

cp "$xfrom" "$xtarget/" || abdialog_fail "Error: Can not copy from [$xfrom] to [$xtarget]."

abdialog_info "Ok, istalled from [$xfrom] to [$xtarget]."

