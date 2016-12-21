#!/bin/bash -e
echo "Entering dir of script $0 (before that, PWD=$PWD)"
cd "${BASH_SOURCE%/*}" || { echo "(error: can not cd into dir)" ; exit 20 ; } # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028

outdir="data/"
echo "This generates the test datafiles, to outdir $outdir (in PWD=$PWD)"

export LC_ALL=C

(

rm -rf "$outdir" ; mkdir "$outdir"
cd "$outdir"

echo "Example small file." > 1.txt

( export m=200 ; for ((i = 0; i < m; ++i)); do echo "Test line $i/$m" ; done )> 2.txt

( export m=1000 ; for ((i = 0; i < m; ++i)); do echo "Test line $i/$m" ; done )> 3.txt

( export m=5000 ; for ((i = 0; i < m; ++i)); do echo "Test line $i/$m" ; done )> 4.txt

( export m=20000 ; for ((i = 0; i < m; ++i)); do echo "Test line $i/$m" ; done )> 5.txt

function make_hashfile() {
	tmpdir="./tmp"
	m="$1"
	rm -rf "$tmpdir"
	mkdir "$tmpdir"
	pwd="$PWD"
	cd "$tmpdir"
	for ((i = 0; i < m; ++i)); do
		echo "Test line $i/$m" > "$i-line.txt"
	done
	sha256sum $(ls -1 | sort -n) > "all.txt"
	cd "$pwd"
	mv "$tmpdir/all.txt" "$2"
	rm -rf "$tmpdir"

	gzip --keep --best --no-name "$2" # create e.g. s1.txt.gz from s1.txt
}

make_hashfile 200 "s1.txt"
make_hashfile 2000 "s2.txt"
make_hashfile 6000 "s3.txt"

sha256sum * > "sha-now.txt" # expected as generated on server

echo "Generated:"
cat "sha-now.txt"
ls -lh

)

echo "If you changed this generation script, then remember to also copy the now-calulated checksums into the good file:"
echo "E.g.:"
echo "cp contrib/ci/autotest/set1/data/sha-now.txt   contrib/ci/autotest/set1/sha-good.txt"

echo "All done in $0."


