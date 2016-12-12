#!/bin/bash -e
export LC_ALL=C # for e.g. sort

function fail {
	echo "Error in $0; " "$@"
	exit 1
}

dir_top="."
[ -r "${dir_top}/toplevel" ] || { echo "Run this (script $0) while being in the top-level directory; Can't find 'toplevel' in PWD=$PWD"; exit 1; }

set -x
project_name="galaxy42"

echo "$0: working on project_name=$project_name ; OUTDIR=$OUTDIR TAR_OPTIONS=$TAR_OPTIONS"

outname="${project_name}-static" # name of project release/output
outname_tgz="${outname}.tar.gz" # name of project release/output
dir_pack_wrap="${dir_top}/pack-linux"
dir_pack="${dir_pack_wrap}/${outname}"
dir_build="${dir_top}"
outfile_tgz="${dir_pack_wrap}/$outname_tgz"

rm -rf "$dir_pack_wrap/" || fail
mkdir -p "$dir_pack/" || fail

fullname_main_binary=$(readlink -f "${dir_build}/tunserver.elf")

cp -i "${dir_build}/tunserver.elf" "${dir_pack}/" || fail
cp -ar "${dir_top}/share" "${dir_pack}/" || fail

dir_static_libs="$dir_pack/staticlibs"
mkdir -p "$dir_static_libs/" || fail
cp /usr/lib/x86_64-linux-gnu/gconv/* "${dir_static_libs}/" || fail

cp "${dir_top}/contrib/add_scripts_build_linux_static/galaxy42-static-start.sh" "${dir_pack}/" || fail

pushd "$dir_pack_wrap" || fail
	rm -f "$outname_tgz"
	echo "Quick sum of all files to be packed:"
	sha1sum $(find "$outname" | sort) | sha1sum -
	echo "Packing, from outname=$outname to $outname_tgz, with TAR_OPTIONS=$TAR_OPTIONS in PWD=$PWD"
	find "$outname" | sort | tar --no-recursion --mode='u+rw,go+r-w,a+X' \
		--owner=0 --group=0 -c \
		-T - | gzip -9n > "${outname_tgz}" || fail "Can not compress"
# gzip -9n
#--mtime="/tmp/faketime_timestamp" \
popd || fail

#echo "Files:"
#find "$dir_pack"

echo "$0: Packed file:"
ls -lh "$outfile_tgz"
sha256sum "$outfile_tgz"

echo "Will copy to $OUTDIR"
cp "$outfile_tgz" "$OUTDIR/" || fail "Can not copy the result to OUTDIR=$OUTDIR"
sha256sum $OUTDIR/* || { echo "Can not calculate checksums... ignoring." ; }

