#!/bin/bash

# run first the script from https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9

function fail() {
	set -x
	printf "%s\n" "$*"
	exit 1
}

if [[ -z "$1" ]] ; then
	echo "Usage:"
	echo "$0 PCK_SIZE THR port_low port_high cardnr 1 2 3 4 ..."
	echo "$0 8972 2  5555     5559      cardnr 1 2 3 4 ...    # will send UDP 8972 B datagrams, in 2 thread/card, on cardnr 1,2,3,4 as configured in p2p9x9 to port 5555..5559"
	echo "$0 8972 2  5555     5555      cardnr 1 2 3 4 ...    # just one port 5555. might be worse for some multiqueue NICs?"
	echo "PCK_SIZE is usually MTU-28, for ipv4 targets, this is size of UDP datagram"
	echo "First run configuration script p2p9x9 ( https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9 )"
	echo
	exit 1
fi

[ "$EUID" -ne 0 ] && fail "You must run this as root."

echo "Loading module"
modprobe pktgen || exit "Can not load module pktgen"

echo "Detecting max threads..."
for ((i = 1; 1 ; i++)); do
	[[ -e "/proc/net/pktgen/kpktgend_$i" ]] || break
done
kpkt_max_thread=$i
echo "number of kpktgen max threads: $kpkt_max_thread"


# set -x

for file in  /proc/net/pktgen/kpktgend_*
do
	echo 'rem_device_all' >> "$file"
done

function prepare_sending() {
	card="$1"
	target="$2"
	threads="$3"
	port_low="$4"
	port_high="$5"
	send_start="$6"
	mtu="$7"
	mac="$8"

	echo "TESTING: on device [$card], will send to target [$target] (MAC $mac) UDP port $port_low...$port_high, using threads [$threads], with MTU=$mtu (UDP datagram size actually)"

	# set -x
	for (( i=0; $i < $threads; i++ )) ; do
		num=$(( send_start + i ))
		send="${card}@${num}"
		num_thread=$(( num % kpkt_max_thread ))
		kpkt_thread_file="/proc/net/pktgen/kpktgend_$num_thread"
		echo "... [$send] via thread file [$kpkt_thread_file]"
		echo "add_device $send" >> "$kpkt_thread_file"  || fail "Can not add_device"
		echo "count 0" >> "/proc/net/pktgen/$send"
		# echo "clone_skb 100" >> "/proc/net/pktgen/$send"
		# echo "frags 1" >> "/proc/net/pktgen/$send"
		# echo "flows 10" >> "/proc/net/pktgen/$send"
		#echo "flowlen 100000" >> "/proc/net/pktgen/$send"
		echo "min_pkt_size $mtu" >> "/proc/net/pktgen/$send"
		echo "max_pkt_size $mtu" >> "/proc/net/pktgen/$send"
		echo "udp_dst_min $port_low" >> "/proc/net/pktgen/$send"
		echo "udp_dst_max $port_high" >> "/proc/net/pktgen/$send"
		echo "dst_min $target" >> "/proc/net/pktgen/$send"
		echo "dst_max $target" >> "/proc/net/pktgen/$send"
		echo "dst_mac $mac" >> "/proc/net/pktgen/$send" # MAC address
	done
	# set +x
}

begin=0
mtu="$1" # this is UDP datagram size actually
thr="$2"
port_low="$3"
port_high="$4"
mode="$5"
shift ; shift ; shift ; shift ; shift

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

	# convert this_targetip (own IP address on interface) into it's MAC address
	this_mac=""
	while IFS= read -r arp_line; do
		IFS=' ' read -r arp_ip arp_hwtype arp_mac other_fields <<< "$arp_line"
		if [[ "$arp_ip" == "$this_targetip" ]] ; then
			this_mac="$arp_mac"
			[[ $arp_hwtype == "(incomplete)" ]] && this_mac="" # delete MAC from incomplete (anyway collumns are then missing)
			break
		fi
	done < <(
		arp -n
	)

	if [[ -z "$this_mac" ]] ; then
		echo "=== WARNING === can not find MAC addr (from arp) for $this_targetip on $this_dev"
	else
		echo "$this_targetip on $this_dev has MAC addr $this_mac"
	fi

	prepare_sending "${this_dev}" "${this_targetip}" "$thr" "$port_low" "$port_high" "$begin" "$mtu" "$this_mac"
	begin=$((begin+thr))
done


echo "Starting the send - NOW... (you can press ctrl-C to exit)"
echo "start" >> /proc/net/pktgen/pgctrl
echo "done."


