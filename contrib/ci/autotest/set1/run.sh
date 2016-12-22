#!/bin/bash
echo "Entering dir of script $0 (before that, pwd=$PWD)"
cd "${BASH_SOURCE%/*}" || { echo "(error: can not cd into dir)" ; exit 20 ; } # cd into the bundle and use relative paths; http://mywiki.wooledge.org/BashFAQ/028

function fail() {
	printf "\n%s\n" "ERROR (in $0): $@"
	exit 1
}

default_peer1="192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"
peer1_reference="${test_galaxy_peer1:-$default_peer1}"

path_to_run="../../../../" # where is our binary

echo "Making tempdir"
tempdir=$(mktemp -d test.XXXXXXXXX -p '') || fail "Can not make tmpdir=$tmpdir"
chmod 700 "$tempdir" || fail "Can not chmod our tmpdir=$tmpdir"
function script_cleanup_tmp() {
	rm -rf "$tempdir"
}
trap script_cleanup_tmp EXIT

echo "Starting server..."
(
	cd "${path_to_run}"
	echo "Running in PWD=$PWD"
	set -x
	nohup ./tunserver.elf &
	# --peer "$peer1_reference" &
	# || { set +x ; echo "WARNING: server exited with error: $?" ; exit 1; }
	child_pid="$!"
	echo "Server is on child_pid=$child_pid"
	set +x
	child_file="$tempdir/server.pid"
	echo "$child_pid" > "$tempdir/server.pid" || { echo "Can not save server pid to file $child_file" ; exit 1; }
	echo "Saved server pid $child_pid to file $child_file."
) &
sleep 1
child_file="$tempdir/server.pid"
if [[ ! -r "$child_file" ]] ; then fail "Can not read the server PID from $child_file. Probably server failed to start at all." ; fi
myserver_pid=$(cat "$child_file")
echo "Our server should be on PID $myserver_pid"

function script_cleanup_pid() {
	pid=$myserver_pid
	if (($pid)) ; then
		printf "%s\n%s" "Exiting: killing process $pid"
		kill "$pid"
		sleep 0.1
		kill -9 "$pid"
		myserver_pid=0
	fi
}
trap script_cleanup_pid EXIT

kill -0 "$myserver_pid" || fail "Server is not alive at PID $myserver_pid"

# --- server probably is alive now, at that PID

echo "Starting the test:"

./test.sh || fail "Test failed"

echo "Test done - seems OK"

kill "$myserver_pid"
sleep 0.1
kill -0 "$myserver_pid" 2>/dev/null && { echo "Still alive? Killing it -9 then." ; kill -9 "$myserver_pid"; }
myserver_pid=0
echo "Normal cleanup done, myserver_pid=$myserver_pid"

