#!/bin/bash

readme_bench='README-bench.md'
echo "See documentation in $readme_bench" # <---

port1=9011
port2=9012
port3=9013
port4=9014

# Use a root dir for this script. http://mywiki.wooledge.org/BashFAQ/028
# - cd into this script's directory
# - and access bundle using $bundle_dir as specified by $relative_top

relative_top="../../" # <--- this is how THIS ONE our script is bundled inside our repo
echo "bash file: [$BASH_SOURCE]" ;
my_src="${BASH_SOURCE}"
[[ "$my_src" == "" ]] && { echo "Unknown script file name. This script must be run from a file (because it is part of bundle)" ; exit 1; }
[[ $my_src && $my_src != */* ]] && my_src=./$my_src
echo "my_src [$my_src]"
cd_to="${my_src%/*}"
echo "cd into [$cd_to]"
cd "$cd_to" || { echo "Can not cd into my script dir [$cd_to]" ; exit 1 ; }
bundle_dir="${PWD}/${relative_top}"
[ -r "${bundle_dir}toplevel" ] || { echo "Can't find 'toplevel' using bundle_dir=[$bundle_dir]"; exit 1; }
source "${bundle_dir}share/script/lib/fail.sh" || { echo "Can not load lib (in $0)"; exit 1 ; }
source "${bundle_dir}share/script/lib/simple-status.sh" || fail "Can not load lib";

echo "OK, working in PWD=[$PWD] with bundle_dir=[$bundle_dir]"

status_title "Will start all tests"

[[ -z "$YEDINO_BENCH_CONFIGURED" ]] && fail "Please configure your system for automatic benchmarks, see file allbench.txt"

echo "This will be running all bench tests"

echo "RPC password will be: [$YEDINO_BENCH_RPC_PASS1] (without the brackets)"

echo "System info...:"
source ./getinfo.sh
echo ""

echo "Will run tests on ($cpu_all_2)"

status_hints_start
echo "Make sure, that following programs are run on helper-fast computer: (see doc $readme_bench)"
for port in $port1 $port2 $port3 $port4 ; do
	echo './asio_send_block remote ' $YEDINO_BENCH_PEER_FAST_RPC_IP ' ' $port '  0.0.0.0       $((5*24*60*60))  $YEDINO_BENCH_RPC_PASS1'
done
echo "And configure there password (as you have here, probably in your ~/.bashrc)"
status_hints_end

this_test="Fast_wire"
status_title "Test: $this_test"
set -x
./test_speed_packet_size.py \
	--cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP1:9000,$YEDINO_BENCH_PEER_FAST_RPC_IP:9011 \
	--cmd sameboth,sameip:9012 \
	--cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP2:9000,sameip:9013 \
	--cmd sameboth,sameip:9014  \
	--ranges 2  --passwd "$YEDINO_BENCH_RPC_PASS1" \
	|| fail "Test failed (test: $this_test)"
set +x



echo "All tests done in $my_src"


