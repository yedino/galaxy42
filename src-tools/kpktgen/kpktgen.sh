#!/bin/bash

# run first the script from https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9

echo "Loading module"
modprobe pktgen || exit 1

set -x

for file in  /proc/net/pktgen/kpktgend_*
do
	echo 'rem_device_all' >> "$file"
done



function prepare_sending() {
	card="$1"
	target="$2"
	threads="$3"
	send_start="$4"

	echo "target=$target"
	for (( i=0; $i <= $threads; i++ )) ; do
		num=$(( send_start + i ))
		send="${card}@${num}"
		echo "add_device $send" >> "/proc/net/pktgen/kpktgend_$num"
		echo "count 0" >> "/proc/net/pktgen/$send"
		# echo "clone_skb 100" >> "/proc/net/pktgen/$send"
		# echo "frags 1" >> "/proc/net/pktgen/$send"
		echo "flows 10" >> "/proc/net/pktgen/$send"
		#echo "flowlen 100000" >> "/proc/net/pktgen/$send"
		echo "min_pkt_size 8972" >> "/proc/net/pktgen/$send"
		echo "max_pkt_size 8972" >> "/proc/net/pktgen/$send"
		echo "udp_dst_min 5550" >> "/proc/net/pktgen/$send"
		echo "udp_dst_max 5559" >> "/proc/net/pktgen/$send"
		echo "dst_min $target" >> "/proc/net/pktgen/$send"
		echo "dst_max $target" >> "/proc/net/pktgen/$send"
	done
}

thr=2
begin=0

prepare_sending "eth6" $(cat /var/local/iplab2/ip-nic-1.txt) "$thr" "$begin"
begin=$((begin+thr))
prepare_sending "eth7" $(cat /var/local/iplab2/ip-nic-2.txt) "$thr" "$begin"
begin=$((begin+thr))


echo "starting..."

echo "start" >> /proc/net/pktgen/pgctrl

echo "done?"


