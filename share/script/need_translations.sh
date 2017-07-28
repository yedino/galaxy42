#!/bin/bash

echo "First, will prepare translations of texts for this script..."
bash contrib/tools/galaxy42-lang-update-all "SCRIPTS_INSTALL" || {
	echo
	echo "Warning: can not prepare translations for this script. Will continiue without them."
	echo "To see translations in this script, install in your system the tools for gettext, tools like:"
	echo "  msgfmt xgettext gettext msgmerge msgattrib"
	echo ""
	echo "Usually with command like:"
	echo "  apt-get install gettext"
	echo "  or on Mac OS X system: brew install gettext on Mac OS X"
	echo "  or on Windows with Cygwin: select the tool gettext on install or update"
	echo ""
	delay=30
	read -p "Press ENTER to continue (or wait $delay seconds and script will continue)... " -t "$delay" _
}
echo
