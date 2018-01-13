
#!/usr/bin/env bash

# http://linuxcommand.org/lc3_adv_tput.php

function status_title() {
	echo -e "\n\n"
	echo "$(tput setaf 6)"
  echo -e "======================================================"
	printf "%s\n" "$@"
  echo -e "======================================================"
	echo -e "\n\n"
	echo "$(tput sgr0)"
}

function status_hints_start() {
	echo "$(tput setab 6)" # cyan
	echo "$(tput setaf 7)" # white
	echo "$(tput bold)"
}

function status_hints_end() {
	echo "$(tput sgr0)"
}
