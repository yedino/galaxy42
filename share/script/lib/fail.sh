#!/usr/bin/env bash

function fail() {
	set +x
	echo "$(tput setaf 1)" || :
	echo -e "\nERROR (in $0): " "$@"
	exit 1
	echo "$(tput sgr0)" || :
}
