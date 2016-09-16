#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# Various utils
# including platform-detection utils

# TODO - FOR HACKING / DEVELOPMENT:
#
# Support detection of more platforms, see places marked: __add_platform__
#


inarray() { local n=$1 h; shift; for h; do [[ $n = "$h" ]] && return; done; return 1; }


# platforminfo - informations about current platform, system, for compatybility

function platforminfo_check_program() {
	hash "$1" 2>/dev/null && echo 1 || echo 0
}


# This variable platforminfo (assoc array) is initialized by call to init_platforminfo
# then you should do some form of: if (( ! platforminfo[family_detected] )) ; then echo "Unknown platform" ; exit ; fi
# after this use:
# ${platforminfo[id]} for 'debian' 'ubuntu' 'devuan' 'fedora' etc.
# ${platforminfo[idver]} for 'debian7' 'debian8' 'ubuntu15.10' etc.
# if (($platforminfo[is_apt])) then do apt-get; also is_dnf, is_yum
# if (($platforminfo[is_family_debian])) then install packages as for Debians family.
# ${platforminfo[packager]} for 'apt-get' 'aptitude' 'yum' etc
# ${platforminfo[packager_cmd_pause]} will be '1' if you need to wait for user confirmation
# also OTHER global variables are set:
# $platforminfo_packager_cmd for array like ('aptitude' 'PACKAGES' '-y') .
# ... also is_family_fedora

declare -A platforminfo

function init_platforminfo() {
	platforminfo[info_lsb]=$(platforminfo_check_program 'lsb_release')

	platforminfo[distro]="not_recognized"
	platforminfo[code]="not_recognized"

	if [[ -r /etc/os-release ]] ; then
		osrelease="$(egrep  'NAME|VERSION|VERSION_ID|ID' /etc/os-release)"
		platforminfo[distro]=$(printf "%s\n" "${osrelease}" | egrep '^NAME=' | sed -e "s/^NAME=\(.*\)/\1/g" | tr '[:upper:]' '[:lower:]'  )
		platforminfo[id]=$(printf "%s\n" "${osrelease}" | egrep '^ID=' | sed -e "s/^ID=\(.*\)/\1/g")
		platforminfo[code]="${platforminfo[id]}"
		platforminfo[only_verid]=$(printf "%s\n" "${osrelease}" | egrep '^VERSION_ID=' | sed -e "s/^VERSION_ID=\(.*\)/\1/g")
	else
		if (( platforminfo['info_lsb'] )) ; then
			lsb=$(lsb_release -a)
			platforminfo[distro]=$( printf "%s\n" "$lsb" | sed -n -e 's/^Distributor ID:[\t ]*\(.*\)/\1/p' )
			platforminfo[id]=$( printf "%s\n" "${platforminfo[distro]}" | tr '[:upper:]' '[:lower:]' ) # "debian" from "Debian"
			platforminfo[code]=$( printf "%s\n" "$lsb" | sed -n -e 's/^Codename:[\t ]*\(.*\)/\1/p' )
			platforminfo[release]=$( printf "%s\n" "$lsb" | sed -n -e 's/^Release:[\t ]*\(.*\)/\1/p' ) # e.g. "8.5"
			platforminfo[only_verid]=$( printf "%s\n" "${platforminfo[release]}" | cut -d'.' -f1 ) # "8" from "8.5"
		else
			printf "Can not read os-release nor lsb_release, can not detect this system at all. Aborting."
			exit 1
		fi
	fi
	platforminfo[idver]="${platforminfo[id]}_${platforminfo[only_verid]}"

	platforminfo[is_apt]=$(platforminfo_check_program 'apt-get') # e.g. debian
	platforminfo[is_aptitude]=$(platforminfo_check_program 'aptitude') # e.g. debian
	platforminfo[is_yum]=$(platforminfo_check_program 'yum') # e.g. fedora
	platforminfo[is_dnf]=$(platforminfo_check_program 'dnf') # e.g. fedora
	platforminfo[is_apk]=$(platforminfo_check_program 'apk') # alpine linux
	# __add_platform__ detect your platform's package manager if it uses something else

	family_detected=0 # not detected yet

	# detect family from the lsb/os-release data
	platforminfo[is_family_redhat]=0
	if [[ ${platforminfo[id]} = @(fedora|redhat|centos) ]] ; then platforminfo[is_family_redhat]=1 ; family_detected=1 ; fi # TODO more distros here? is redhat and centos good?

	platforminfo[is_family_debian]=0
	if [[ ${platforminfo[id]} = @(debian|devuan|ubuntu|mint) ]] ; then platforminfo[is_family_debian]=1 ; family_detected=1 ; fi # TODO more distros here? is redhat and centos good?

	platforminfo[is_family_debian]=0
	if [[ ${platforminfo[id]} = @(alpine) ]] ; then platforminfo[is_family_alpine]=1 ; family_detected=1 ; fi

	# __add_platform__ if your platform is in above family then add it to the test by name,
	# if it is a new family then copy entire block of course
	# echo "DEVELOPMENT, SYS ID: ${platforminfo[id]}"

	if (( ! family_detected )) ; then # can not detect family directly
		# try detecting family from packager
		if ((platforminfo[is_apt]))      ; then platforminfo[is_family_debian]=1; family_detected=1 ; fi
		if ((platforminfo[is_aptitude])) ; then platforminfo[is_family_debian]=1; family_detected=1 ; fi
		if ((platforminfo[is_yum]))      ; then platforminfo[is_family_fedora]=1; family_detected=1 ; fi
		if ((platforminfo[is_dnf]))      ; then platforminfo[is_family_fedora]=1; family_detected=1 ; fi
		if ((platforminfo[is_apk]))      ; then platforminfo[is_family_alpine]=1; family_detected=1 ; fi
		# __add_platform__ if your platform is a new family that can be detected by used packager then add it here
	fi

	platforminfo[packager]="unknown"
	platforminfo[packager_cmd_pause]=0 # most commands do not need a pause

	if (( platforminfo[is_apt] )) ; then
		platforminfo[packager]='apt'
		platforminfo_packager_cmd=('apt' 'install' '-y' 'PACKAGES')
	fi
	if (( platforminfo[is_aptitude] )) ; then
		platforminfo[packager]='aptitude'
		platforminfo_packager_cmd=('aptitude' 'install' '-y' 'PACKAGES')
	fi
	if (( platforminfo[is_yum] )) ; then
		platforminfo[packager]='yum'
		platforminfo_packager_cmd=('yum' 'install' '-y' 'PACKAGES')
	fi
	if (( platforminfo[is_dnf] )) ; then
		platforminfo[packager]='dnf'
		platforminfo_packager_cmd=('dnf' 'install' '-y' 'PACKAGES')
	fi
	if (( platforminfo[is_apk] )) ; then
		platforminfo[packager]='apk'
		platforminfo_packager_cmd=('apk' 'add' 'PACKAGES')
	fi

	if [[ "${platforminfo[packager]}" == "unknown" ]] ; then
		platforminfo_packager_cmd=('echo' 'Please install following packages (or similar depending on you OS/platform) in your system (e.g. use other console and then come back or start again): ' 'PACKAGES')
		platforminfo[packager_cmd_pause]=1
	fi

	if (( ! "$family_detected" )) ; then printf "%s\n" "Warning: detect does not recognize this OS/platform type that is used here (IdVer=$platforminfo['idver'])." ; fi ;
	platforminfo[family_detected]="$family_detected" # yes or no
}

function platforminfo_show_all() {
	for K in "${!platforminfo[@]}"; do printf "%s" "$K = ${platforminfo[$K]}\n"; done
}

function platforminfo_show_summary() {
	printf "%s" "${platforminfo[id]}"
	printf "%s" " (${platforminfo[idver]})"
	printf "\n%s" "Packager: ${platforminfo[packager]}"
	printf "\n%s" "Flags:"
	for ik in "${!platforminfo[@]}"; do
		iv="${platforminfo[$ik]}"
		if [[ $ik == is_* ]] ; then # for keys of format is_....
			if ((iv)) ; then # if it's value is true...
				printf "%s" " $ik" ;
			fi
		fi
	done
	printf "."
	printf "\n"
}

function platforminfo_install_packages() {
	declare -a cmdparam
	cmd=""
	cmd_taken=0

	echo "Called util install packages with:" "$@"

	for part in "${platforminfo_packager_cmd[@]}" ; do
		if (( ! $cmd_taken )) ; then
			cmd="$part"
			cmd_taken=1
		else			
			if [[ "$part" == "PACKAGES" ]] ; then
				for arg in "$@" ; do
					cmdparam+=( "$arg" )
				done
			else
				cmdparam+=( "$part" )
			fi
		fi
	done

	echo "Will now call installer, command is: $cmd" ;	for part in "${cmdparam[@]}" ; do echo "With command line argument: $part" ; done  # debug
	echo "Calling now."
	$cmd "${cmdparam[@]}" || return $?
}

function platforminfo_test() {
	printf "Test platforminfo_test\n"
	if ((platforminfo[is_dnf])) ; then printf "Can use DNF.\n" ; fi
	if ((platforminfo[is_yum])) ; then printf "Can use YUM.\n" ; fi
	if ((platforminfo[is_apt])) ; then printf "Can use APT.\n" ; fi
	if ((platforminfo[is_apk])) ; then printf "Can use APK.\n" ; fi
	if ((platforminfo[is_xxx])) ; then printf "Using XXX (huh?).\n" ; fi
	printf "End of test platforminfo_test\n"
}



