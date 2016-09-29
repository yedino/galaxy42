#!/bin/bash
printf "Running gitian configuration (the SETTINGS) for USER=$USER on HOSTNAME=$HOSTNAME (this should be run inside Gitian)"

theline='export TERM="xterm"' >> ~/.bashrc
thefile="$HOME/.bashrc"

function fail() {
	printf "\n\n%s\n\\n" "Error in script $0 (${BASH_SOURCE}), running on USER=$USER on HOSTNAME=$HOSTNAME:" "$@"
	exit 1
}

grep -q "$theline" "$thefile" || { echo "$theline" >> "$thefile" || fail "Can not write to file $thefile" ; }

