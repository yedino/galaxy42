#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# Various utils
# including platform-detection utils
#
# g42bashutils - the galaxy42's bash utils (created for Galaxy42 project, but usable for anything else too)
#

# TODO - FOR HACKING / DEVELOPMENT:
#
# Support detection of more platforms, see places marked: __add_platform__
#


inarray() { local n=$1 h; shift; for h; do [[ $n = "$h" ]] && return; done; return 1; }
join_by() { local IFS="$1"; shift; echo "$*"; }


# platforminfo - informations about current platform, system, for compatybility

function platforminfo_check_program() {
	hash "$1" 2>/dev/null && echo 1 || echo 0
}

# This variable platforminfo (assoc array) is initialized by call to init_platforminfo
# then you should do some form of: if (( ! platforminfo[family_detected] )) ; then echo "Unknown platform" ; exit ; fi
# after this use:
# ${platforminfo[id]} for 'debian' 'ubuntu' 'devuan' 'fedora' etc.
# ${platforminfo[is_sudo]} it is possible to maybe use sudo here.
# ${platforminfo[idver]} for 'debian7' 'debian8' 'ubuntu15.10' etc.
# if (($platforminfo[is_apt])) then do apt-get; also is_dnf, is_yum
# if (($platforminfo[is_family_debian])) then install packages as for Debians family.
# ${platforminfo[packager]} for 'apt-get' 'aptitude' 'yum' etc
# ${platforminfo[packager_cmd_pause]} will be '1' if you need to wait for user confirmation
# also OTHER global variables are set:
#
# $platforminfo_packager_install_cmd - command to install packages, format: array like ('aptitude' 'PACKAGES' '-y') .
# $platforminfo_packager_checkinstalled_cmd - command (array) to check IF package is installed
# $platforminfo_packager_remove_cmd - command (array) to REMOVE a package
#
# ... also is_family_fedora

declare -A platforminfo

function init_platforminfo() {
	platforminfo[info_lsb]=$(platforminfo_check_program 'lsb_release')

	platforminfo[distro]="not_recognized"
	platforminfo[code]="not_recognized"

	if [[ -r /etc/os-release ]] ; then
		osrelease="$(egrep  'NAME|VERSION|VERSION_ID|ID' /etc/os-release)"
		platforminfo[distro]=$(printf "%s\n" "${osrelease}" | egrep '^NAME=' | sed -e "s/^NAME=\(.*\)/\1/g" \
			| tr '[:upper:]' '[:lower:]' | tr -cd '[[:alnum:]]._-' )
		platforminfo[id]=$(printf "%s\n" "${osrelease}" | egrep '^ID=' | sed -e "s/^ID=\(.*\)/\1/g" \
			| tr -cd '[[:alnum:]]._-' )
		platforminfo[code]="${platforminfo[id]}"
		platforminfo[only_verid]=$(printf "%s\n" "${osrelease}" | egrep '^VERSION_ID=' | sed -e "s/^VERSION_ID=\(.*\)/\1/g" \
			| tr -cd '[[:alnum:]]._-' )
	else
		if (("platforminfo['info_lsb']")) ; then
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
	echo "Utils detect platforminfo: idver=${platforminfo[idver]}"

	platforminfo[is_sudo]=$(platforminfo_check_program 'sudo') # can we use sudo if we are not running as root
	platforminfo[is_now_root]=0
	if [[ $EUID -eq 0 ]]; then platforminfo[is_now_root]=1 ; fi

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

	platforminfo[is_family_alpine]=0
	if [[ ${platforminfo[id]} = @(alpine) ]] ; then platforminfo[is_family_alpine]=1 ; family_detected=1 ; fi

	# __add_platform__ if your platform is in above family then add it to the test by name,
	# if it is a new family then copy entire block of course
	# echo "DEVELOPMENT, SYS ID: ${platforminfo[id]}"

	if (( ! "family_detected" )) ; then # can not detect family directly
		# try detecting family from packager
		if (("platforminfo[is_apt]"))      ; then platforminfo[is_family_debian]=1; family_detected=1 ; fi
		if (("platforminfo[is_aptitude]")) ; then platforminfo[is_family_debian]=1; family_detected=1 ; fi
		if (("platforminfo[is_yum]"))      ; then platforminfo[is_family_fedora]=1; family_detected=1 ; fi
		if (("platforminfo[is_dnf]"))      ; then platforminfo[is_family_fedora]=1; family_detected=1 ; fi
		if (("platforminfo[is_apk]"))      ; then platforminfo[is_family_alpine]=1; family_detected=1 ; fi
		# __add_platform__ if your platform is a new family that can be detected by used packager then add it here
	fi

	platforminfo[packager]="unknown"
	platforminfo[packager_cmd_pause]=0 # most commands do not need a pause

	if (("platforminfo[is_apt]")) ; then
		platforminfo[packager]='apt'
		platforminfo_packager_install_cmd=('apt' 'install' '-y' 'PACKAGES')
		platforminfo_packager_remove_cmd=('apt' 'remove' '-y' 'PACKAGES')
		platforminfo_packager_checkinstalled_cmd=('./contrib/tools/debian_package_is_installed' 'PACKAGE')
	fi
	if (("platforminfo[is_aptitude]")) ; then
		platforminfo[packager]='aptitude'
		platforminfo_packager_install_cmd=('aptitude' 'install' '-y' 'PACKAGES')
		platforminfo_packager_remove_cmd=('aptitude' 'remove' '-y' 'PACKAGES')
		platforminfo_packager_checkinstalled_cmd=('./contrib/tools/debian_package_is_installed' 'PACKAGE')
	fi
	if (("platforminfo[is_yum]")) ; then
		platforminfo[packager]='yum'
		platforminfo_packager_install_cmd=('yum' 'install' '-y' 'PACKAGES')
		platforminfo_packager_remove_cmd=('yum' 'remove' '-y' 'PACKAGES')
		platforminfo_packager_checkinstalled_cmd=('yum' 'list' 'installed' 'PACKAGE')
	fi
	if (("platforminfo[is_dnf]")) ; then
		platforminfo[packager]='dnf'
		platforminfo_packager_install_cmd=('dnf' 'install' '-y' 'PACKAGES')
		platforminfo_packager_remove_cmd=('dnf' 'remove' '-y' 'PACKAGES')
		platforminfo_packager_checkinstalled_cmd=('dnf' 'list' 'installed' 'PACKAGE')
	fi
	if (("platforminfo[is_apk]")) ; then
		platforminfo[packager]='apk'
		platforminfo_packager_install_cmd=('apk' 'add' 'PACKAGES')
		# TODO unknown
	fi

	if [[ "${platforminfo[packager]}" == "unknown" ]] ; then
		platforminfo_packager_install_cmd=('echo' 'Please install following packages (or similar depending on you OS/platform) in your system (e.g. use other console and then come back or start again): ' 'PACKAGES')
		platforminfo_packager_remove_cmd=('echo' 'Please REMOVE following packages (or similar depending on you OS/platform) in your system (e.g. use other console and then come back or start again): ' 'PACKAGES')
		platforminfo[packager_cmd_pause]=1
		# TODO provide a test here
	fi

	if (( ! "$family_detected" )) ; then printf "%s\n" "Warning: detect does not recognize this OS/platform type that is used here (IdVer=${platforminfo['idver']})." ; fi ;
	platforminfo[family_detected]="$family_detected" # yes or no

	# printf "DETECTED item=${platforminfo_packager_checkinstalled_cmd[@]}\n"  #XXX
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

function run_with_root_privilages() {
	if (( ! "platforminfo[is_now_root]" )) ; then
		if (( "platforminfo[is_sudo]" )) ; then
			printf "\n%s\n" "We can use sudo to run as root (OK)"
			printf "\n"
			cmd="$1" ; shift
			set -x
			sudo "$cmd" "$@" || return $?
			set +x
			printf "\n"
		else
			printf "%s\n" "ERROR: Not root now, and sudo is not available - can not install with root rights then."
			return 50
		fi
	else
		printf "\n%s\n" "We are root (OK)"
		printf "\n"
		cmd="$1" ; shift
		set -x
		$cmd "$@" || return $?
		set +x
		printf "\n"
	fi
}

function platforminfo_install_packages() {
	declare -a cmdparam
	cmd=""
	cmd_taken=0

	echo "Called util install packages with:" "$*"

	for part in "${platforminfo_packager_install_cmd[@]}" ; do
		if (( ! "cmd_taken" )) ; then
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

	echo -n "Will now call packager, command is: $cmd, args:" ;	for part in "${cmdparam[@]}" ; do echo -n "$part | " ; done  # debug
	echo

	run_with_root_privilages "$cmd" "${cmdparam[@]}" || return $?
}

function platforminfo_remove_packages() {
	declare -a cmdparam
	cmd=""
	cmd_taken=0

	echo "Called util REMOVE packages with:" "$*"

	for part in "${platforminfo_packager_remove_cmd[@]}" ; do
		if (( ! "cmd_taken" )) ; then
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

	echo -n "Will now call packager, command is: $cmd, args:" ;	for part in "${cmdparam[@]}" ; do echo -n "$part | " ; done  # debug
	echo

	run_with_root_privilages "$cmd" "${cmdparam[@]}" || return $?
}

function platforminfo_checkinstalled_package() {
	declare -a cmdparam
	cmd=""
	cmd_taken=0

	echo
	echo "Called util CHECK if installed package(s?) with:" "$*"
	echo

	for part in "${platforminfo_packager_checkinstalled_cmd[@]}" ; do
		if (( ! "cmd_taken" )) ; then
			cmd="$part"
			cmd_taken=1
		else
			if [[ "$part" == "PACKAGE" ]] ; then # one PACKAGE.
				for arg in "$@" ; do
					cmdparam+=( "$arg" )
				done
			else
				cmdparam+=( "$part" )
			fi
		fi
	done

	echo -n "Will now call packager, command is: $cmd, args:" ;	for part in "${cmdparam[@]}" ; do echo -n "$part | " ; done  # debug

	# run_with_root_privilages
	if "$cmd" "${cmdparam[@]}" ; then
		printf "\n%s\n\n" "Test of package returns TRUE, it IS INSTALLED."
		true
	else
		printf "\n%s\n\n" "Test of package returns FALSE, it IS NOT installed."
		false
	fi
}


function platforminfo_test() {
	printf "Test platforminfo_test\n"
	if (("platforminfo[is_dnf]")) ; then printf "Can use DNF.\n" ; fi
	if (("platforminfo[is_yum]")) ; then printf "Can use YUM.\n" ; fi
	if (("platforminfo[is_apt]")) ; then printf "Can use APT.\n" ; fi
	if (("platforminfo[is_apk]")) ; then printf "Can use APK.\n" ; fi
	if (("platforminfo[is_xxx]")) ; then printf "Using XXX (huh?).\n" ; fi # test
	printf "End of test platforminfo_test\n"
}


#
# (quiet|verbose) the_directory (fix|dryrun) "good1,good2,...,goodN" "bad1,bad2,...badN"
# $1              $2            $3           $4                      $5
# eg:
# platforminfo_set_mountflags verbose '/home/rafalcode/' fix 'dev,exec' 'nodev,noexec'
#
# Will make sure that given the_directory has all of good flags, and none of the bad flags
# (or just report the problem if option 'dryrun'.
# @return exit code 0 if all is fine (e.g. fixed now or was already good), 1 is you must call mount, 2 if error
# @return via global variable $g42utils_resulting_mount_args - list of arguments to mount (or empty)
#

declare -a g42utils_resulting_mount_args
export g42utils_resulting_mount_args

function platforminfo_set_mountflags() {
	g42utils_resulting_mount_args=()

	# export is for shellcheck
	local oldlca=$LC_ALL; export oldlca; trap 'LC_ALL=$oldlca' 0 ; LC_ALL="C"
	local oldtd=$TEXTDOMAIN; export oldtd; trap 'TEXTDOMAIN=$oldtd' 0 ; TEXTDOMAIN="g42bashutils"

	verbose=0
	if [[ "$1" == 'verbose' ]] ; then verbose=1 ;
	else
		if [[ "$1" == 'quiet' ]] ; then verbose=0 ;
		else
			printf "Unknown flag (internal error)\n"; return 50;
		fi
	fi
	targetdir="$2" # the dir to check

	fix=0

	printf "%s\n" "Checing mount options of file-system that contains $targetdir."
	if [[ "$3" == 'fix' ]] ; then fix=1 ;
	else
		if [[ "$3" == 'dryrun' ]] ;
			then fix=0 ;
		else
			printf "Unknown flag (internal error)\n"; return 50;
		fi
	fi
	export fix # shellcheck yes it is unused.

	mapfile -t flagsGood < <(printf "%s" "$4" | sed -e 's|,|\n|g')
	mapfile -t flagsBad  < <(printf "%s" "$5" | sed -e 's|,|\n|g')

	mountdir=$( df -h "$targetdir" | tail -n +2 | sed -r -e 's|[\t ]+|;|g' | cut -d';' -f 6 )
	[ -z "$mountdir" ] && { printf "%s\n" "$(eval_gettext "Can not find where \$targetdir is mounted.")" ; return 10 ; }

	(( verbose )) && printf "%s\n" "We will check file-system mounted at $mountdir."

	fsflags_str=$( mount  | egrep  "on $mountdir type " | cut -d' ' -f 6 | sed -e 's|[()]||g' )
	(( verbose )) && printf "%s\n" "fs flags read as [$fsflags_str]"


	mapfile -t flags < <(printf "%s\n" "$fsflags_str" | sed -e 's|,|\n|g')
	(( verbose )) && printf "%s\n" "The file-system $mountdir has ${#flags[@]} flag(s): [${flags[*]}]"

	declare -A flag_negate
	flag_negate['dev']='nodev'
	flag_negate['nodev']='dev'
	flag_negate['exec']='noexec'
	flag_negate['noexec']='exec'

	toadd=()

	# Decide what to change. E.g:
	# flags: nodev,atime,noexec; good=dev; bad=noexec ---> need=dev,exec
	for flagGood in "${flagsGood[@]}" ; do
		inarray "$flagGood" "${flags[@]}" || {
			(( verbose )) && printf "%s\n" "Need to add good flag [$flagGood]."
			toadd+=("$flagGood")
		}
	done

	# (( verbose )) && printf "bad: %s\n" "${flagsBad[@]}"

	for flagBad in "${flagsBad[@]}" ; do
		# printf "Testig bad: $flagBad in ${flags[*]}\n"
		inarray "$flagBad" "${flags[@]}" && {
			negate="${flag_negate[$flagBad]}"
			(( verbose )) && printf "%s\n" "Need to remove unwanted flag $flagBad - by adding flag [$negate]."
			toadd+=("$negate")
		}
	done

	declare -A addclear
	printf "%s\n" "Need to add flags count: ${#toadd[@]}"
	for i in "${toadd[@]}"; do addclear[$i]=1; done
	if (( ${#toadd[@]} == 0 )) ; then
		printf "%s\n" "All is ok, nothing needs to be fixed on ${mountdir}."
		return 0
	fi

	new_flags="remount,"$( join_by ',' "${!addclear[@]}" )
	# TODO add the option -i only when needed, e.g. only for encfs:
	g42utils_resulting_mount_args=( "$mountdir" -i -o "$new_flags" ) # the mount command
	printf "%s\n" "Need to remount ${mountdir} by using: ${g42utils_resulting_mount_args[*]}"
	return 1
}

# parameter is megabytes per thread
function pfp_util_estimate_make_threads() {
       local threads=1
       if [[ -r /proc/meminfo ]]; 
       then
               mem_per_thread=$1
               memavaiable=$( cat /proc/meminfo | grep 'MemAvailable' | awk -F " " '{print $2}' )
               memavaiable=$( echo "${memavaiable}/1024" | bc )
               num_cores=$( cat /proc/cpuinfo | grep processor | wc -l )
               num_threads_from_ram=$( echo "${memavaiable}/${mem_per_thread}" | bc )
               num_cores=$( expr ${num_cores} + 1 )
               if [ ${num_threads_from_ram} -ge 0 ];
               then
                       if [ ${num_threads_from_ram} -gt ${num_cores} ];
                       then
                               echo "Limiting number of threads due to CPU cores, from ${num_threads_from_ram} to ${num_cores} (memory per thread needed is assumed:      ${mem_per_thread} MB)" >&2
                               threads=${num_cores}
                       else
                               echo "Limiting number of threads due to ${num_threads_from_ram} (memory per thread needed is assumed: ${mem_per_thread} MB)" >&2
                               threads=${num_threads_from_ram}
                       fi
               fi
       fi
       echo ${threads}
}

