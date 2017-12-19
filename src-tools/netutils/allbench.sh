#!/bin/bash

# Use a root dir for this script. http://mywiki.wooledge.org/BashFAQ/028
dir_base_of_source="../../" # <--- this is how THIS ONE our script is bundled inside our repo
echo "bash file: [$BASH_SOURCE]" ; cd_to="${BASH_SOURCE%/*}" ; cd "$cd_to" || { echo "Can not cd into my script dir [$cd_to]" ; exit 1 ; }
echo "Script works in PWD [$PWD]"
dir_base_of_source="${PWD}/${dir_base_of_source}/"
[ -r "${dir_base_of_source}toplevel" ] || { echo "Can't find 'toplevel' using dir_base_of_source=[$dir_base_of_source]"; exit 1; }
source "${dir_base_of_source}share/script/lib/fail.sh" || { echo "Can not load lib (in $0)"; exit 1 ; }
echo "OK"

exit

echo "This will be running all bench tests"

echo "RPC password will be: $YEDINO_BENCH_RPC_PASS1"

echo "System info..."
source ./getinfo.sh
echo ""

echo "Will run tests on ($cpu_all_2)"

./test_speed_packet_size.py --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP1:9000,$YEDINO_BENCH_PEER_FAST_RPC_IP:9011 --cmd sameboth,sameip:9012 --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP2:9000,sameip:9013 --cmd sameboth,sameip:9014  --ranges 2  --passwd $YEDINO_BENCH_RPC_PASS1

echo "All tests done"


