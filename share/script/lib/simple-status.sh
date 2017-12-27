
#!/usr/bin/env bash

function status_title() {
	echo -e "\n\n"
	echo "$(tput setaf 6)"
  echo -e "======================================================"
	printf "%s\n" "$@"
  echo -e "======================================================"
	echo -e "\n\n"
	echo "$(tput sgr0)"
}
