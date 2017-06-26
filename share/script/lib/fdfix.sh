#!/usr/bin/env bash

function fdfix() {
	echo "Applying fix for /dev/fd" # lxc lacks /dev/fd that is needed e.g for bash < <(....) redirection
	sudo ln -s /proc/self/fd /dev/fd || { echo "Can not fix fd. Maybe not needed, ignoring." ; }
}

