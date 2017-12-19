#!/bin/bash

# Use a root dir for this script. http://mywiki.wooledge.org/BashFAQ/028
relative_top="../../" # <--- this is how THIS ONE our script is bundled inside our repo
echo "bash file: [$BASH_SOURCE]" ;

my_src="${BASH_SOURCE}"
echo "my_src [$my_src]"
[[ "$my_src" == "" ]] && { echo "Unknown script file name. This script must be run from a file (because it is part of bundle)" ; exit 1; }
echo "my_src [$my_src]"
[[ $my_src && $my_src != */* ]] && my_src=./$my_src
echo "my_src [$my_src]"
cd_to="${my_src%/*}"
echo "cd into [$cd_to]"
cd "$cd_to" || { echo "Can not cd into my script dir [$cd_to]" ; exit 1 ; }
bundle_dir="${PWD}/${relative_top}"
[ -r "${bundle_dir}toplevel" ] || { echo "Can't find 'toplevel' using bundle_dir=[$bundle_dir]"; exit 1; }
source "${bundle_dir}share/script/lib/fail.sh" || { echo "Can not load lib (in $0)"; exit 1 ; }
echo "OK, working in bundle_dir [$bundle_dir]"

exit

echo "This will be running all bench tests"

echo "RPC password will be: $YEDINO_BENCH_RPC_PASS1"

echo "System info..."
source ./getinfo.sh
echo ""

echo "Will run tests on ($cpu_all_2)"

./test_speed_packet_size.py --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP1:9000,$YEDINO_BENCH_PEER_FAST_RPC_IP:9011 --cmd sameboth,sameip:9012 --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP2:9000,sameip:9013 --cmd sameboth,sameip:9014  --ranges 2  --passwd $YEDINO_BENCH_RPC_PASS1

echo "All tests done"


