#!/bin/bash

echo "This will be running all bench tests"

echo "RPC password will be: $YEDINO_BENCH_RPC_PASS1"

echo "System info..."
source ./getinfo.sh
echo ""

echo "Will run tests on ($cpu_all_2)"

./test_speed_packet_size.py --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP1:9000,$YEDINO_BENCH_PEER_FAST_RPC_IP:9011 --cmd sameboth,sameip:9012 --cmd $YEDINO_BENCH_SELF_FAST_WIRE_IP2:9000,sameip:9013 --cmd sameboth,sameip:9014  --ranges 2  --passwd $YEDINO_BENCH_RPC_PASS1

echo "All tests done"


