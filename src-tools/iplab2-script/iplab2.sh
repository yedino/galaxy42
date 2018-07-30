#!/bin/bash
# iplab2 - Copyrighted (C) 2018 rfree ; BSD 2-clause licence
# Quick code, might be insecure
# See help_usage() for information

set -o errexit
set -o nounset

function fail() { echo "Error: $@" ; exit 1; }

readonly config_global="/etc/iplab2/global.sh" # <--- runs [as root] script ! must be read-only secure location
source "$config_global" || fail "Failed to run global config script [$config_global]"

# ^--- put there data that describes your computer/test-LAN, copy/paste, uncomment all lines except 1st, and edit:
#/bin/bash
# readonly ipBlock="10"
# readonly ipNetmask="24"
# readonly ipVarDir="/var/local/iplab2/"
# # this computer:
# readonly ipMyBase=4
# declare -a ipCARD=("enp1s0f0" "enp1s0f1")
# ---

function help_usage() {
	echo ""
	echo "Script to quickly configure your test NICs to connect to other computers/NICs in up to 9x9 test lab network."
	echo ""
	echo "Setup:"
	echo "Create file like /etc/iplab2/global.sh (see this script for exact path and copy/paste template of config)"
	echo ""
	echo "Usage:"
	echo 'program "1 100 3:1" "2 100 4:1" "3 101 4:9"'
	echo 'program "connect1" "connect2" "connect3" ...'
	echo 'connect is: "my_nic world target_base:target_nic"'
	echo 'connect "2 100 4:9" means: my 2-nd card, in normal world (100) goes to 4-th computer into his 9-th card'
	echo 'connect "3 101 4:8" means: my 3-rd card, in extra  world (101) goes to 4-th computer into his 8-th card'
	echo "world should be 100, but use 101 etc if more then one connection from you to SAME target computer (to allow route selection)"
	echo "all computer numbers, and card numbers, should be 1..9 ."
	echo "program -h or --help shows help"
	echo ""
}

ipTARGET=( "$@" )

[[ -z "$*" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "-h" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "--help" ]] && { help_usage ; exit 40 ; }

echo "---------------------"
echo "Your cards (configured in $config_global)"
ix=1
for target in "${ipCARD[@]}"
do
	dev=${ipCARD[ix-1]} # -1 to move to 0-based bash array indexes
	echo "Your cardnr :$ix is $dev"
	ix=$((ix+1))
done
echo "---------------------"
echo

[[ "$ipMyBase" -gt 9 ]] && fail "Too big ipMyBase ($ipMyBase)"
[[ -z "$ipVarDir" ]] && fail "Configure ipVarDir."
[[ "$ipVarDir" =~ ^/[a-zA-Z0-9/]{1,100}/iplab2/$ ]] || fail "Invalid configuration ipVarDir (regex)"

[[ -w "${ipVarDir}" ]] || {
	mkdir -p "${ipVarDir}" || fail "Can not create dir $ipVarDir"
}

for ifile in ${ipVarDir}ip-*.txt
do
	[[ -e "$ifile" ]] || break
	rm --preserve-root "${ifile}"
done
chmod a+rX "${ipVarDir}/"

for target in "${ipTARGET[@]}"
do
	target2=$( echo "$target" | sed -e's/:/ /g')
	read my_nicnr our_world target_base target_nicnr <<< "$target2" || fail "Parse error in $[target]"
	[[ "$my_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in $[target] ($my_nicnr) my_nicnr"
	[[ "$our_world" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in $[target] ($our_world) our_world"
	[[ "$target_base" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in $[target] ($target_base) target_base"
	[[ "$target_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in $[target] ($target_nicnr) target_nicnr"

	[[ "$my_nicnr" -gt 9 ]] && fail "Too big my_nicnr ($my_nicnr)"
	[[ "$our_world" -gt 220 ]] && fail "Too big our_world ($our_world)"
	[[ "$target_base" -gt 9 ]] && fail "Too big target_base ($target_base)"
	[[ "$target_nicnr" -gt 9 ]] && fail "Too big target_nicnr ($target_nicnr)"

	# if I am base 4, and target is 7 then our common net is .74. sorted, so .47.
	c1="$ipMyBase"
	c2="$target_base"
	[[ "$c1" -gt "$c2" ]] && { tmp="$c1" ; c1="$c2" ; c2="$tmp"; }
	[[ "$c1" == "$c2" ]] && fail "Trying to connect to yourself? You are base ipMyBase=$ipMyBase , target base = $target_base"
	commonNet="$c1$c2"

	my_endip=$(( ipMyBase*10 + my_nicnr ))
	target_endip=$(( target_base*10 + target_nicnr ))

	# echo "Will connect to computer base=$target_base to his nic_nr=$target_nicnr, common net=.${commonNet}."
	my_ip="${ipBlock}.${our_world}.${commonNet}.${my_endip}"
	target_ip="${ipBlock}.${our_world}.${commonNet}.${target_endip}"

	my_eth="${ipCARD[my_nicnr-1]}"
	echo "My $my_ip (:$my_nicnr=$my_eth) ---to----> his $target_ip (:$target_nicnr)"

	#set -x
	ip address flush dev ${my_eth}
	ip link set dev ${my_eth} up
	ip addr add "${my_ip}/${ipNetmask}" dev "$my_eth"
	ifconfig "${my_eth}" mtu 9000
	#set +x

	ping -w 1 -W 1 -c 1 "$target_ip" > /dev/null && echo "^--- PING OK ($target_ip)"
	echo ""

	echo "${target_ip}" > "${ipVarDir}/ip-${my_nicnr}.txt"
done

echo ""
echo "---------------------"
echo "See IPs in files in $ipVarDir (use in scripts etc)"
for ifile in ${ipVarDir}ip-*.txt
do
	[[ -e "$ifile" ]] || break
	echo "${ifile}:"
	cat "${ifile}"
done


echo "Done"


