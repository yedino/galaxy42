#!/bin/bash
# This code (in this file) is on BSD 2-clause license, (c) Copyrighted 2016 TiguSoft.pl
# Various utils
# including platform-detection utils
#
# galaxy42-middle-utils - the Middle utils for galaxy42. This are tools that are quite closely related to galaxy-42,
# and are not generic enough tobe part of utils.sh
#

#
# sets some variables so that caller can act on them:
#
# sets global variable $midutils_apt_cacher_version to "old" or "ng",
# sets global variable $midutils_apt_cacher_version_name_good to "apt-cacher" or "apt-cacher-ng" (install and run this package)
# sets global variable $midutils_apt_cacher_version_name_service to "apt-cacher" or "apt-cacher-ng" (start this service)
# sets global variable $midutils_apt_cacher_version_name_bad  to "apt-cacher" or "apt-cacher-ng" (remove this package)
#
export midutils_apt_cacher_version
export midutils_apt_cacher_version_name_good
export midutils_apt_cacher_version_name_bad
export midutils_apt_cacher_version_name_service

function midutils_detect_correct_apt_cacher_version() {
	# related to bug #J202
	# most systems want apt-cacher-ng and not old apt-cacher. but there are exceptions
	printf "\n\n\napt-cacher selection\n\n"
	local apt_cacher='ng'

	if [[ "${platforminfo[distro]}" == "ubuntu" ]]; then
		# get ubuntu main version e.g. "14" from "ubuntu_14.04"
		local ubuntu_ver=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
		# ubuntu_ver_minor=$( echo "${platforminfo[only_verid]}" | cut -d'.' -f1)
		if (( ubuntu_ver <= 14 )); then apt_cacher='ng'; fi
	fi

	(( verbose && apt_cacher!='ng' )) && {
		show_fix "$(eval_gettext "For this system we selected apt-cacher type: \$apt_cacher")"
	}

	case "$apt_cacher" in
		'ng')
			local apt_cacher_bad='apt-cacher'
			local apt_cacher_good='apt-cacher-ng'
			local apt_cacher_service='apt-cacher-ng'
		;;
		'old')
			local apt_cacher_bad='apt-cacher-ng'
			local apt_cacher_good='apt-cacher'
			local apt_cacher_service='apt-cacher'
		;;
		*)
			fail "Internal error: unknown apt_cacher type."
		;;
	esac

	midutils_apt_cacher_version="$apt_cacher"
	midutils_apt_cacher_version_name_good="$apt_cacher_good"
	midutils_apt_cacher_version_name_bad="$apt_cacher_bad"
	midutils_apt_cacher_version_name_service="$apt_cacher_service"
}


