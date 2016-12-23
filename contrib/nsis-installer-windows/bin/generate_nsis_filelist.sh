#!/usr/bin/env bash

set -o errexit
set -o nounset

readonly GALAXY42_DIR="$HOME/build/galaxy42"
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

is_dir() {
	local dir="$1"
	[[ -d "$dir" ]]
}

gen_x86_64() {

	if ( is_dir "${SCRIPT_DIR}/x64" ); then
		find ${SCRIPT_DIR}/x64 -regex ".*\.\(dll\|exe\)" \
			| sort \
			| awk -F "x64/" '{print "\t\tFILE bin/x64/"$2}'
			
	else 
		echo "Can't find x64 dir in $SCRIPT_DIR, skipping..."
	fi
}
gen_i686() {
	
	if ( is_dir "${SCRIPT_DIR}/x86" ); then
		find ${SCRIPT_DIR}/x86 -regex ".*\.\(dll\|exe\)" \
			| sort \
			| awk -F "x86/" '{print "\t\tFILE bin/x86/"$2}'

	else 
		echo "Can't find x86 dir in $SCRIPT_DIR, skipping..."
	fi

}
gen_noarch() {

	if ( is_dir "${SCRIPT_DIR}/noarch" ); then
		echo -e '\t# translation file'
		echo
		# static creating directories for four languages em pl ru and uk
		echo -e '\tCreateDirectory "$INSTDIR\share\locale\\en\LC_MESSAGES\"'
		echo -e '\tCreateDirectory "$INSTDIR\share\locale\\pl\LC_MESSAGES\"'
		echo -e '\tCreateDirectory "$INSTDIR\share\locale\\ru\LC_MESSAGES\"'
		echo -e '\tCreateDirectory "$INSTDIR\share\locale\\uk\LC_MESSAGES\"'
		echo

		# getting path's of language files
		declare -a path_list=()
		while IFS=  read -r -d $'\0'; do
			path_list+=("$REPLY")
		done < <(find ${SCRIPT_DIR}/noarch -regex ".*.LC_MESSAGES.*.mo" -print0 | sort -z)
		
		# pasing path to get nsis record example:  /.../rest_of_path/share/locale/en/LC_MESSAGES/g42bashutils.mo  -->
		# FILE /oname=$INSTDIR\share\locale\en\LC_MESSAGES\g42bashutils.mo	bin/noarch/share/locale/en/LC_MESSAGES/g42bashutils.mo
		
		for path in "${path_list[@]}"; do
			local filename=$( echo "${path}" | awk -F "noarch/" '{print $2}' )
			local win_format_file=$(echo "$filename" | sed 's/\//\\/g')
			printf "\tFILE /oname=\$INSTDIR\\%s\tbin/noarch/%s\n" "${win_format_file}" "${filename}"
		done
	else 
		echo  -e '\t\t' | echo "Can't find noarch dir in $SCRIPT_DIR, skipping..."
	fi
}

main() {

	echo
	echo -e '\t${If} ${RunningX64}'
	gen_x86_64
	echo -e '\t${Else}'
	gen_i686
	echo -e '\t${EndIf}'
	echo
	gen_noarch
	echo

}
main
