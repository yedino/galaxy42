#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# platforminfo - informations about current platform, system, for compatybility
lsb=$(lsb_release -a)

function platforminfo_check_program() {
	hash "$1" 2>/dev/null && echo 1 || echo 0
}

declare -A platforminfo
platforminfo[distro]=$( printf "$lsb" | sed -n -e 's/^Distributor ID:[\t ]*\(.*\)/\1/p' )
platforminfo[code]=$( printf "$lsb" | sed -n -e 's/^Codename:[\t ]*\(.*\)/\1/p' )
platforminfo[is_apt]=$(platforminfo_check_program 'apt-get')
platforminfo[is_yum]=$(platforminfo_check_program 'yum')

function platforminfo_show_all() {
	for K in "${!platforminfo[@]}"; do printf "$K = ${platforminfo[$K]}\n"; done
}

