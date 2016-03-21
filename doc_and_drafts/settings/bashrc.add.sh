#!/bin/bash
# add this to END of your ~/.bashrc
# cat thisfile >> ~/.bashrc

# --- tigusoft hacks for bashrc ---

# ----> EDIT the lines marked [CONFIG] if you want to <----

export PATH="$HOME/.local/bin:/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games"

export TZ="/usr/share/zoneinfo/UTC"

(( BASH_VERSINFO[0] == 4 && BASH_VERSINFO[1] >= 3 || BASH_VERSINFO[0] > 4 )) && {
	_readline_git_toplevel() { 
		local -n r=READLINE_LINE i=READLINE_POINT
		local dir;
		local text;
		dir=$(git rev-parse --show-toplevel 2>/dev/null) || return 0
		text="${dir}/" # the text to add
		r=${r:0:i}$text${r:i}
		(( i += ${#text} ))
	}
	bind -x '"\C-g":"_readline_git_toplevel"'
}


EDITOR='vim';
export EDITOR

function edit_kws() {
	echo "Killing whitespaces on $1"
	sed -i 's/[ \t]*$//' "$1"
}

xset r rate 160 60


function __ps_netif_glaxy {
	# 17: galaxy0: <POINTOPOINT,
	netif_galaxy=$( ip a | egrep 'galaxy[0-9]+:' | sed -e 's/.*galaxy\([0-9]\+\):.*/\1/g' )
	netif_galaxy_count=$( ip a | egrep 'galaxy[0-9]+:' | sed -e 's/.*galaxy\([0-9]\+\):.*/\1/g' | wc -l )
	[[ $netif_galaxy_count == "1" ]] && {
		echo -n -e '\e[44m\e[33m'
		echo -n " galaxy $netif_galaxy "
		# lame? way to extract ipv6 address: 
		# ip a | grep galaxy -A 10 | grep inet6 | head -n 1 | sed 's/[ ]*inet6 \([0-9a-f:]\+\)\/.*$/\1/g'
		addr=$( ip a | grep galaxy -A 10 | grep inet6 | head -n 1 | sed 's/[ ]*inet6 \([0-9a-f:]\+\)\/.*$/\1/g' )
		echo -n -e '\e[32m'
		echo -n "$addr "
	}
}

function __ps_netif_privnet {
	# TODO make this hacked code nicer
	# prva2xG@if8:
	netif_guest=$( ip a | egrep 'prv.+x?@if' | sed -e 's/.*prv\(.\+\).G@.*/\1/g' )
	netif_guest_count=$( ip a | egrep 'prv.+x?@if' | sed -e 's/.*prv\(.\+\).G@.*/\1/g' | wc -l )
	[[ $netif_guest_count == "1" ]] && {
		# echo "You are in guest network [$netif_guest]"
		echo -n -e '\e[35m'
		echo "<guest $netif_guest>"
	}
}

# [CONFIG] remove the ending $ from your default PS above, or use this one instead (uncomment next 1 line):
PS1='${debian_chroot:+($debian_chroot)}\[\033[01;30m\][$(date +%Y-%m-%d_%H:%M:%S)] \[\033[01;38m\]\u@\[\033[01;34m\]\h\[\033[00m\]:\[\033[01;32m\]\w\[\033[01;33m\]($(__git_ps1 "%s"))'

PS1="${PS1}\$(__ps_netif_privnet)\$(__ps_netif_glaxy)"
PS1="${PS1}"'\[\033[00m\]\$ '


