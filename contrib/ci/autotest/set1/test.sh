#!/bin/bash
echo "Entering dir of script $0 (before that, pwd=$PWD)"
cd "${BASH_SOURCE%/*}" || { echo "(error: can not cd into dir)" ; exit 20 ; } # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028

vardir="var/"
rm -rf "$vardir" ; mkdir "$vardir" ; cd "$vardir"

opt_baseurl="http://[fd42:f6c4:9d19:f128:30df:b289:aef0:25f5]/meshnet/autotest/"
timeout=10
filelist="../sha-good.txt"
echo "Starting test (in PWD=$PWD), expected data is listed in $filelist, and we will download to $vardir, from url <$opt_baseurl>"

while read -r item_checksum_expected item_filename; do
	url="${opt_baseurl}${item_filename}"

	printf '%s\n' "Downloading: $url"

	wget --quiet --tries 1 --timeout "$timeout" "$url" -O "${item_filename}" # <------

	item_checksum_now=$(sha256sum "${item_filename}" | cut -f1 -d' ' )
	item_size_now=$(stat --printf="%s" "${item_filename}")
	echo -n "Checksum: $item_checksum_now - "
	if [[ "$item_checksum_now" == "$item_checksum_expected" ]] ; then
		echo "OK (size: $item_size_now b)"
	else
		echo "FAILED (size: $item_size_now b)"
		printf '%s\n\n' "Invalid checksum, now=$item_checksum_now vs expected=$item_checksum_expected on file url=$url."
		exit 1 # XXX
	fi
done < "$filelist"


rm -rf "$vardir"
echo "All done in $0."

