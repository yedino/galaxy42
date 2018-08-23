#!/bin/bash

# run first the script from https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9

function fail() {
	set -x
	printf "%s\n" "$*"
	exit 1
}

if [[ -z "$1" ]] ; then
	echo "Usage:"
	echo "$0 MTU THR cardnr 1 2 3 4 ..."
	echo "$0 8972 2 cardnr 1 2 3 4 ...    # will send 8972 B, in 2 thread/card, on cardnr 1,2,3,4 as configured in p2p9x9"
	echo "First run configuration script p2p9x9 ( https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9 )"
	echo
	exit 1
fi

[ "$EUID" -ne 0 ] && fail "You must run this as root."

echo "Loading module"
modprobe pktgen || exit "Can not load module pktgen"

# set -x

for file in  /proc/net/pktgen/kpktgend_*
do
	echo 'rem_device_all' >> "$file"
done

function prepare_sending() {
	card="$1"
	target="$2"
	threads="$3"
	send_start="$4"
	mtu="$5"

	echo "TESTING: on device [$card], will send to target [$target] using threads [$threads], with MTU=$mtu (UDP)"

	# set -x
	for (( i=0; $i <= $threads; i++ )) ; do
		num=$(( send_start + i ))
		send="${card}@${num}"
		echo "add_device $send" >> "/proc/net/pktgen/kpktgend_$num" || fail "Can not add_device"
		echo "count 0" >> "/proc/net/pktgen/$send"
		# echo "clone_skb 100" >> "/proc/net/pktgen/$send"
		# echo "frags 1" >> "/proc/net/pktgen/$send"
		echo "flows 10" >> "/proc/net/pktgen/$send"
		#echo "flowlen 100000" >> "/proc/net/pktgen/$send"
		echo "min_pkt_size $mtu" >> "/proc/net/pktgen/$send"
		echo "max_pkt_size $mtu" >> "/proc/net/pktgen/$send"
		echo "udp_dst_min 5555" >> "/proc/net/pktgen/$send"
		echo "udp_dst_max 5555" >> "/proc/net/pktgen/$send"
		echo "dst_min $target" >> "/proc/net/pktgen/$send"
		echo "dst_max $target" >> "/proc/net/pktgen/$send"
	done
	# set +x
}

begin=0
mtu="$1"
thr="$2"
mode="$3"
shift ; shift ; shift ;

echo "Config: mtu=$mtu threads=$thr"

[[ "$mode" == "cardnr" ]] || fail "Wrong mode. Run without arguments to see usage."

while true ;
do
	cardnr="$1"
	[[ -z "$cardnr" ]] && break;
	shift

	echo "Will send through your cardnr=$cardnr (from p2p9x9 configuration)"
	this_dev=""
	this_targetip=""
	this_dev=$(cat "/var/local/iplab2/nic${cardnr}-dev.txt") || fail "Can not read p2p9x9 config"
		this_targetip=$(cat "/var/local/iplab2/nic${cardnr}-dstip.txt") || fail "Can not read p2p9x9 config"

	prepare_sending "${this_dev}" "${this_targetip}" "$thr" "$begin" "$mtu"
	begin=$((begin+thr))
done


echo "Starting the send - NOW... (you can press ctrl-C to exit)"

echo "start" >> /proc/net/pktgen/pgctrl

echo "done."


