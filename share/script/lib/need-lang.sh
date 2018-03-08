#!/bin/bash

source gettext.sh || { echo "Gettext is not installed, please install it." ; exit 1 ; }

source "${dir_base_of_source}share/script/need_translations.sh"

function init_lang() {
	echo "init_lang()..."
	supported_lang=(en_US.UTF-8 pl_PL.UTF-8 uk_UA.UTF-8 ru_RU.UTF-8 de_DE.UTF-8)
	lang_default='en_US.UTF-8'

	lang="$LANG"
	[[ -z "$lang" ]] && lang="$LANGUAGE"
	[[ -z "$lang" ]] && lang="$LC_ALL"

	echo "init_lang (start) - lang=[$lang] from LANG=($LANG) LANGUAGE=($LANGUAGE) LC_ALL=($LC_ALL)"

	lang_ok=0
	case "${supported_lang[@]}" in  *"$lang"*) lang_ok=1 ;; esac

	if [[ "$lang_ok" == "1" ]] ; then
		echo "lang OK ($lang)"
	else
		echo "lang not supported ($lang)"
		lang="$lang_default"
		LANG="$lang"
		LC_ALL=""
		LANGUAGE=""
		echo  "lang changed to default: [$lang]"
	fi
	echo "init_lang()... DONE"
	echo "init_lang (final) - lang=[$lang] from LANG=($LANG) LANGUAGE=($LANGUAGE) LC_ALL=($LC_ALL)"
}

