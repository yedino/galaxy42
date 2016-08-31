#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# platforminfo - informations about current platform, system, for compatybility
lsb=$(lsb_release -a)

function platforminfo_check_program() {
	hash "$1" 2>/dev/null && echo 1 || echo 0
}

declare -A platforminfo
platforminfo[distro]=$( printf "%s" "$lsb" | sed -n -e 's/^Distributor ID:[\t ]*\(.*\)/\1/p' )
platforminfo[code]=$( printf "%s" "$lsb" | sed -n -e 's/^Codename:[\t ]*\(.*\)/\1/p' )
platforminfo[is_apt]=$(platforminfo_check_program 'apt-get')
platforminfo[is_yum]=$(platforminfo_check_program 'yum')
platforminfo[is_dnf]=$(platforminfo_check_program 'dnf')

function platforminfo_show_all() {
	for K in "${!platforminfo[@]}"; do printf "%s" "$K = ${platforminfo[$K]}\n"; done
}

function platforminfo_show_summary() {
	printf "%s" "Distro=${platforminfo[distro]}"
	printf "%s" " Code=${platforminfo[code]}"
	printf "%s" " Flags:"
	for ik in "${!platforminfo[@]}"; do
		iv="${platforminfo[$ik]}"
		if [[ $ik == is_* ]] ; then
			if ((iv)) ; then
				printf "%s" " $ik" ;
			fi
		fi
	done
	printf "."
	printf "\n"
}


