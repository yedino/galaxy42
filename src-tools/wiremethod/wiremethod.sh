#!/bin/bash

debug=0 # <--- set to 1 for some debug also run with method 1

echo "Runs network wire speeds"
echo "for Blitz or similar computer"
echo "REMEMBER: turn off HT (each odd CPU core) first!"
echo "see/edit script source (hardcoded values!!)"

function fail() {
	echo "Error: $*"
	exit 1
}

# progs_dir="$HOME/programs/"
# cd "$progs_dir"|| fail "Can not enter $progs_dir . Build the tools from yedino and place there the binaries"
PATH="$PWD:$PATH"

pwd
ls

g_pids=( )

function finish() {
	echo "Killing all jobs: " "${g_pids[@]}"
	kill -9 "${g_pids[@]}"
}


function start_program_() {
	program_bin="$1" ; shift
	task_cpu="$1" ; shift
	cmd_taskset=""
	if [[ ! -z "$task_cpu" ]] ; then
		cmd_taskset="taskset -c $task_cpu" # task_cpu is not quoted, this all forms one long string of 1st command before main cmd
	else
		cmd_taskset=""
	fi

	if [[ "$debug" == 1 ]] ; then
		set -x
		taskset -c "$task_cpu" "$program_bin" "$@"   & p="$!"
		set +x
		sleep 1
	else
		taskset -c "$task_cpu" "$program_bin" "$@" &>/dev/null  & p="$!"
	fi

	task_cpu_s=$(printf "%10s" "$task_cpu")
	echo "Started program $program_bin on CPU [$task_cpu_s] CMD: [$*] to PID=$p"
	g_pids+=("$p")
	#echo "Pids after: " "${g_pids[@]}"
}

function convert_devname_to_mac() {
	# not yet used?
	ip a | egrep -e '^[[:digit:]]*:[ ]*'$1':.*' -A 5 | grep  'link/ether' | head -n 1 | sed -e 's|^.* \([0-9a-f:]*\) .*|\1|g'
}

function start_asb() {
	cardselect="$1" ; shift
	cpus="$1" ; shift
	[[ "$debug" == 1 ]] && echo "ASB: cardselect=[$cardselect] cpus=[$cpus] opts=[$*]"
	start_program_ "./asio_send_block" "$cpus" "$(< /var/local/iplab2/$cardselect-dstip.txt)" "$@"  999000    foo   "$msgsize"    -1  "$@"
}

function start_raweth() {
	cardselect="$1" ; shift
	cpus="$1" ; shift

	if [[ "$cardselect" =~ ^dev-.*$ ]] ; then
		device="${cardselect#dev-}"
	else
		device="$(< /var/local/iplab2/$cardselect-dev.txt)"
	fi

	# mymac=$( convert_devname_to_mac "$cardselect" )
	# echo "My mac on [$cardselect] should be [$mymac]."
	[[ "$debug" == 1 ]] && echo "raweth: cardselect=[$cardselect] cpus=[$cpus] opts=[$*]"
	start_program_ "./rawsend.bin" "$cpus" "$device" flood "$msgsize" "$@"
}

function start_multi() {
	cardselect="$1" ; shift
	cpus="$1" ; shift
	[[ "$debug" == 1 ]] && echo "multi: cardselect=[$cardselect] cpus=[$cpus] opts=[$*]"
	start_program_ "./multi.bin" "$cpus" "$(< /var/local/iplab2/$cardselect-dstip.txt)" "$msgsize"  "$@"
}


function start_sendprog() {
	cardselect="$1" ; shift
	cpus="$1" ; shift
	ports="$1" ; shift
	[[ "$debug" == 1 ]] && echo "PROGRAM: cardselect=[$cardselect] cpus=[$cpus] ports=[$ports]"
	if [[ "$sendprog" == "a" ]] ; then
		start_asb "$cardselect" "$cpus" "$ports" "$@"
	elif [[ "$sendprog" == "r" ]] ; then
		start_raweth "$cardselect" "$cpus" "$@"
	elif [[ "$sendprog" == "m" ]] ; then
		start_multi "$cardselect" "$cpus" "$@"
	fi
}


echo -e "\n\nConfigured for computer: \nDragon\n\n\n"
core_per_cpu=1
msgsize=8972
dev1=$(<"/var/local/iplab2/conn1-dev.txt")
dev2=$(<"/var/local/iplab2/conn2-dev.txt")
dev3=$(<"/var/local/iplab2/conn3-dev.txt")
dev4=$(<"/var/local/iplab2/conn4-dev.txt")
echo -e "\n\n"

devtab=("$dev1" "$dev2" "$dev3" "$dev4")
for i in $(seq 0 3) ; do
	echo "device index $i is ${devtab[$i]}"
done

function irq_of_nic() {
	dev="$1"
	irqs=( $( ls $(readlink -e "/sys/class/net/$dev")/../../msi_irqs ) )   # this ( $(ls..) )  is ok since there are no spaces there, just simple numbers
	echo "${irqs[@]}"
}

function apply_aff_show() {
	for i in $(seq 1 4) ;
	do
		# ughhhh. need to change that to array.
		var_dev="dev${i}"
		var_aff="dev${i}aff"
		echo -n "${!var_dev} affinity ${!var_aff}, bin: "
		echo "ibase=16;obase=2;${!var_aff}" | bc
		echo
	done
}

function apply_aff() {
	[[ ! -z "${devafftab[0]}" ]] && dev1aff="${devafftab[0]}"
	[[ ! -z "${devafftab[1]}" ]] && dev2aff="${devafftab[1]}"
	[[ ! -z "${devafftab[2]}" ]] && dev3aff="${devafftab[2]}"
	[[ ! -z "${devafftab[3]}" ]] && dev4aff="${devafftab[3]}"
	echo "Affinities are: [$dev1aff] [$dev2aff] [$dev3aff] [$dev4aff]"
	apply_aff_show

	if [[ ! -z "$dev1aff" ]] ; then
		for irq in $(irq_of_nic "$dev1") ; do  sudo ./iplab2_affinity.sh "$irq" "$dev1aff" ; done
	fi
	if [[ ! -z "$dev2aff" ]] ; then
		for irq in $(irq_of_nic "$dev2") ; do  sudo ./iplab2_affinity.sh "$irq" "$dev2aff" ; done
	fi
	if [[ ! -z "$dev3aff" ]] ; then
		for irq in $(irq_of_nic "$dev3") ; do  sudo ./iplab2_affinity.sh "$irq" "$dev3aff" ; done
	fi
	if [[ ! -z "$dev4aff" ]] ; then
		for irq in $(irq_of_nic "$dev4") ; do  sudo ./iplab2_affinity.sh "$irq" "$dev4aff" ; done
	fi
}

function aff_flag() {
	start="$1" ; shift
	exp="cpunr=$start;obase=16;0"

	for arg in "$@" ; do
		c="$arg"
		exp="${exp} + 2^($c + cpunr*$core_per_cpu)"
	done
	echo "$exp" | bc
}

trap finish EXIT


function start_this_shit {

method="$1" ; shift
sendprog="$1" ; shift
echo "--- Method-$method sendprog=$sendprog (other options: [$*]) ---"

if [[ "$method" == "22" ]] ; then
	dev1aff=$( aff_flag 0    0 1 2 3 4 5 )
	dev2aff=$( aff_flag 0    0 1 2 3 4 5 )
	dev3aff=$( aff_flag 0    0 1 2 3 4 5 )
	dev4aff=$( aff_flag 0    0 1 2 3 4 5 )
	apply_aff
	for i in $(seq 0 2) ; do start_sendprog "conn1"  "0-5"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn2"  "0-5"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn3"  "0-5"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn4"  "0-5"  "$((5550+i))" "$@"; done

elif [[ "$method" == "21" ]] ; then
	dev1aff=$( aff_flag 0    0 )
	dev2aff=$( aff_flag 0    1 )
	dev3aff=$( aff_flag 0    2 )
	dev4aff=$( aff_flag 0    3 )
	apply_aff
	for i in $(seq 0 1) ; do start_sendprog "conn1"  "0"  "$((5550+i))" "$@"; done
	for i in $(seq 0 1) ; do start_sendprog "conn2"  "1"  "$((5550+i))" "$@"; done
	for i in $(seq 0 1) ; do start_sendprog "conn3"  "2"  "$((5550+i))" "$@"; done
	for i in $(seq 0 1) ; do start_sendprog "conn4"  "3"  "$((5550+i))" "$@"; done

elif [[ "$method" == "20" ]] ; then
	dev1aff=$( aff_flag 0    0 )
	dev2aff=$( aff_flag 0    1 )
	dev3aff=$( aff_flag 0    2 )
	dev4aff=$( aff_flag 0    3 )
	dev1=
	apply_aff
	for i in $(seq 0 2) ; do start_sendprog "conn1"  "0"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn2"  "1"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn3"  "2"  "$((5550+i))" "$@"; done
	for i in $(seq 0 2) ; do start_sendprog "conn4"  "3"  "$((5550+i))" "$@"; done

elif [[ "$method" == "16" ]] ; then
	dev1aff=$( aff_flag 0  3 4 5 )
	dev2aff=$( aff_flag 1  3 4 5 )
	apply_aff
	for i in $(seq 0 6) ; do start_sendprog "dev-$dev1"  "$((0+0*6))-$((2+0*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 6) ; do start_sendprog "dev-$dev2"  "$((0+1*6))-$((2+1*6))"  "$((5550+i))" "$@"; done

elif [[ "$method" == "15" ]] ; then
	echo "WARNING --- WIP." ; read _
	for ixdev in $(seq 0 3) ; do
		local ixcpu="$ixdev" ;
		devafftab[$ixdev]=$( aff_flag $ixcpu 0 1 2 3 4 5 )
	done
	apply_aff
	for ixdev in $(seq 2 3) ; do
		local ixcpu="$ixdev" ;
		for i in $(seq 0 6) ; do
			local cardselect="dev-${devtab[$ixdev]}"
			start_sendprog "$cardselect"   "$((0+ixcpu*6))-$((5+ixcpu*6))"  "$((5550+i))" "$@";
		done
	done

elif [[ "$method" == "14" ]] ; then
	# echo "WARNING --- WIP." ; read _
	for ixdev in $(seq 0 3) ; do
		local ixcpu="$ixdev" ;
		devafftab[$ixdev]=$( aff_flag $ixcpu  3 4 5 )
	done
	apply_aff
	for ixdev in $(seq 0 3) ; do
		local ixcpu="$ixdev" ;
		for i in $(seq 0 6) ; do
			local cardselect="dev-${devtab[$ixdev]}"
			start_sendprog "$cardselect"   "$((0+ixcpu*6))-$((2+ixcpu*6))"  "$((5550+i))" "$@";
		done
	done

elif [[ "$method" == "13" ]] ; then
	echo "WARNING --- this all old test had incorrect IRQ-device-conn mapping and also not setting IRQ AFF dev3,dev4. Do not use that now." ; read _
	dev1aff=$( aff_flag 0  0 1 2 3 4 5 )
	dev2aff=$( aff_flag 1  0 1 2 3 4 5 )
	dev3aff=$( aff_flag 2  0 1 2 3 4 5 )
	dev4aff=$( aff_flag 3  0 1 2 3 4 5 )
	apply_aff
	echo -e "\nSending from 2 cards (for X520 device), but 2 are unused (NX3030) - this is how this test was planned.\n"
	sleep 1
#	for i in $(seq 0 5) ; do start_sendprog "conn1"  "$((0+0*6))-$((2+0*6))"  "$((5550+i))" "$@"; done
#	for i in $(seq 0 5) ; do start_sendprog "conn2"  "$((0+1*6))-$((2+1*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 5) ; do start_sendprog "conn3"  "$((0+2*6))-$((2+2*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 5) ; do start_sendprog "conn4"  "$((0+3*6))-$((2+3*6))"  "$((5550+i))" "$@"; done

elif [[ "$method" == "12" ]] ; then
	echo "WARNING --- this all old test had incorrect IRQ-device-conn mapping and also not setting IRQ AFF dev3,dev4. Do not use that now." ; read _
	dev1aff=$( aff_flag 0  3 4 5 )
	dev2aff=$( aff_flag 1  3 4 5 )
	dev3aff=$( aff_flag 2  3 4 5 )
	dev4aff=$( aff_flag 3  3 4 5 )
	apply_aff
	for i in $(seq 0 0) ; do start_sendprog "conn1"  "$((0+0*6))-$((2+0*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 0) ; do start_sendprog "conn2"  "$((0+1*6))-$((2+1*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 0) ; do start_sendprog "conn3"  "$((0+2*6))-$((2+2*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 0) ; do start_sendprog "conn4"  "$((0+3*6))-$((2+3*6))"  "$((5550+i))" "$@"; done

elif [[ "$method" == "11" ]] ; then
	dev1aff=$( aff_flag 0  3 4 5 )
	dev2aff=$( aff_flag 1  3 4 5 )
	apply_aff
	for i in $(seq 0 6) ; do start_sendprog "conn1"  "$((0+0*6))-$((2+0*6))"  "$((5550+i))" "$@"; done
	for i in $(seq 0 6) ; do start_sendprog "conn2"  "$((0+1*6))-$((2+1*6))"  "$((5550+i))" "$@"; done

elif [[ "$method" == "10" ]] ; then
	dev1aff=$( aff_flag 0  0 1 2 3 4 5 )
	dev2aff=$( aff_flag 1  0 1 2 3 4 5 )
	apply_aff
	for i in $(seq 0 3) ; do start_sendprog "conn1"  "$((0+0*6))-$((5+0*6))"  $((5550+i)) ; done
	for i in $(seq 0 3) ; do start_sendprog "conn2"  "$((0+1*6))-$((5+1*6))"  $((5550+i)) ; done

elif [[ "$method" == "9" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 6 )
	dev2aff=$( aff_flag 1  0 2 4 6 )
	apply_aff
	for i in $(seq 0 3) ; do start_sendprog "conn1"  "$((i*2+0*12))"  $((5550+i)) ; done
	for i in $(seq 0 3) ; do start_sendprog "conn2"  "$((i*2+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "8" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 6 8 10 )
	dev2aff=$( aff_flag 1  0 2 4 6 8 10 )
	apply_aff
	for i in $(seq 0 3) ; do start_sendprog "conn1"  "$((0+0*12))-$((11+0*12))"  $((5550+i)) ; done
	for i in $(seq 0 3) ; do start_sendprog "conn2"  "$((0+1*12))-$((11+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "7" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  10 )
	dev2aff=$( aff_flag 1  10 )
	apply_aff
	for i in $(seq 0 5) ; do start_sendprog "conn1"  "$((0+0*12))-$((8+0*12))"  $((5550+i)) ; done
	for i in $(seq 0 5) ; do start_sendprog "conn2"  "$((0+1*12))-$((8+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "6" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 6 8 10)
	dev2aff=$( aff_flag 1  0 2 4 6 8 10)
	apply_aff
	for i in $(seq 0 5) ; do start_sendprog "conn1"  "$((0+0*12))-$((11+0*12))"  $((5550+i)) ; done
	for i in $(seq 0 5) ; do start_sendprog "conn2"  "$((0+1*12))-$((11+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "5" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 )
	dev2aff=$( aff_flag 1  0 2 4 )
	apply_aff
	for i in $(seq 0 2) ; do start_sendprog "conn1"  "$((i*2+0*12))"  $((5550+i)) ; done
	for i in $(seq 0 2) ; do start_sendprog "conn2"  "$((i*2+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "4" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 6 8 10 )
	dev2aff=$( aff_flag 1  0 2 4 6 8 10 )
	apply_aff
	for i in $(seq 1 6) ; do start_sendprog "conn1"  "$((0+0*12))-$((11+0*12))"  $((5550+i)) ; done
	for i in $(seq 1 6) ; do start_sendprog "conn2"  "$((0+1*12))-$((11+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "3" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( aff_flag 0  0 2 4 6 8 10 )
	dev2aff=$( aff_flag 1  0 2 4 6 8 10 )
	apply_aff
	for i in $(seq 1 6) ; do start_sendprog "conn1"  "$((0+0*12))-$((12+0*12))"  $((5550+i)) ; done
	for i in $(seq 1 6) ; do start_sendprog "conn2"  "$((0+1*12))-$((12+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "2" ]] ; then
	echo "WARNING --- this all old test was assuming x12 numbering. it will NOT WORK as before now." ; read _
	dev1aff=$( echo 'obase=16;2^(10+0*12)' | bc )
	dev2aff=$( echo 'obase=16;2^(10+1*12)' | bc )
	apply_aff
	for i in $(seq 1 6) ; do start_sendprog "conn1"  "$((0+0*12))-$((12+0*12))"  $((5550+i)) ; done
	for i in $(seq 1 6) ; do start_sendprog "conn2"  "$((0+1*12))-$((12+1*12))"  $((5550+i)) ; done

elif [[ "$method" == "1" ]] ; then
	echo "Deleted (had speed 15 gbps, see history/git)"


elif [[ "$method" == "0" ]] ; then
	echo "Running 1 just as example/debug"
	start_sendprog "conn1" "0-11" "5555"

fi

echo

apply_aff_show

for i in $(seq 1 3) ; do echo "------------"; done
for i in $(seq 1 1) ; do echo "PRESS ENTER TO STOP" ; done
for i in $(seq 1 3) ; do echo "------------"; done

read _


echo "All done, exiting"

}




start_this_shit "$@"
