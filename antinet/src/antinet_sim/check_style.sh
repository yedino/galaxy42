
sha1sum -c *.sha1sum || {
	echo "One (or more) of the style-tests failed, your editor might be not correctly configured (tabs/spaces) or e.g. git (if it switched line-endings on checkout)"
}

