#!/bin/bash

function gitian_check_net_devices() {
	be_loud=0 ;	[[ "$1" == "loud" ]] && be_loud=1

	dev1="br0"
	dev2="lxcbr0"

	devices=$(ip link show up | egrep "^[0-9]+: ") || true
	found=$(echo "$devices" | egrep "$dev1|$dev2") || true

	if [ -z "$found" ]; then
		if ((be_loud)) ; then printf "\n\n\n\n" ; fi
		printf "WARNING: Not found any running gitian-lxc network device!"
		printf "%s\n" "missing: $dev1 or $dev2 or both"
		if ((be_loud)) ; then printf "\n\nLXC network will probably NOT WORK right now...\n\n" ; fi
	else
		printf "%s\n" "Found running gitian-lxc network devices, good:"
		printf "%s\n" "$found"
	fi
}

function gitian_config_specific_platforms() {

	# setting devices for ubuntu <= 14
	if [[ "${platforminfo[distro]}" == "ubuntu" ]]; then
		# get ubuntu main version e.g. "14" from "ubuntu_14.04"
		ubuntu_ver=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
		# if ubuntu main version is older/equal than 14
		if (( $ubuntu_ver <= 14 )); then
			sudo ifconfig lxcbr0 down   
		fi
	fi  
}
 
