#!/bin/bash
# iplab2 - Copyrighted (C) 2018 rfree ; BSD 2-clause licence
# Warning: Quick code for tests, might be insecure !!!
# See help_usage() or run --help for information and purpose of this script.

# set -o errexit
# set -o nounset

function fail() { echo "Error: $*" ; exit 1; }

function help_usage() {
	echo ""
	echo "Script to quickly configure your test NICs to connect to other computers/NICs in up to 9 computer x 9 cards test lab network."
	echo "Choose a base number 1..9 for each of your computers. Then install (configure) this script on each computer"
	echo "When you run this script on each computer, telling it the connections from given one computer to others, then all cards on all computers end up with properly configured IPs and networks"
	echo ""
	echo "Script assigns IPs to cards. All cards are point-to-point connections from one computer to another."
	echo "Computers: Alice(6) Bob(7)"
	echo "Alice(6):1 --- Bob(7):2 (Computer alice base number 6 connects her card 1 into computer Bob base number 7 his card 2)"
	echo "Alice(6):1 --- Bob(7):2 will get IPs: 10.67.12.61 --- 10.67.12.72"
	echo "Alice(6):2 --- Bob(7):1 will get IPs: 10.67.21.62 --- 10.67.21.71"
	echo "For information, script assigns IP by following rules:"
	echo "looking at example 10.67.21.62 above, each segment of this IP is calculated as:"
	echo "  10. is always the prefix"
	echo " .67. is made from base numbers of the two computers, first digit is the smaller number (6<7) as digit, then the other"
	echo " .21. is made from CARD numbers on both sides, first digits is CARD NUMBER (2) from computer with smaller BASE number (6<7 therefore take 2), then the other"
	echo " .62  is made from this computer IP number as first digit, then it's card number. (and for remote IP it's remote's base and then it's card number)"
	echo ""
	echo "Setup:"
	echo "Create file like /etc/iplab2/global.sh (see this script for exact path and copy/paste template of config)"
	echo ""
	echo "Usage:"
	echo 'program "1 100 3:1" "2 100 4:1" "3 101 4:9"'
	echo 'program "connect1" "connect2" "connect3" ...'
	echo 'connect is: "my_nic target_base:target_nic"'
	echo 'connect "2 4:9" means: my 2-nd card, goes to 4-th computer into his 9-th card'
	echo 'connect "3 4:8" means: my 3-rd card, goes to 4-th computer into his 8-th card'
	echo "  my_nic and target_nic - the card numbers, on each computer, are numbered from 1 (so 1,2,3...) and order depends given computer's configuration ipCARD variable. Allowed range is 1..9."
	echo "  target_base is the base-number of given computer (as he configured in his configuration ipMyBase variable). Allowed range is 1..9/"
	echo "program -h or --help shows help"
	echo ""
}
[[ -z "$*" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "-h" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "--help" ]] && { help_usage ; exit 40 ; }

readonly config_global="/etc/iplab2/global.sh" # <--- runs [as root] script ! must be read-only secure location
source "$config_global" || fail "Failed to run global config script [$config_global] - read my source code for instructions how to create it; Run me with --help to read instructions."

# Installation:
# ^--- create directory and file as above and put there collowing few lines (uncomment all lines except 1st) and edit this settings to your computer
#!/bin/bash
# readonly ipBlock="10"
# readonly ipNetmask="24"
# readonly ipVarDir="/var/local/iplab2/"
# # this computer:
# readonly ipMyBase=4 # <--- put here base-number of current computer
# declare -a ipCARD=("enp1s0f0" "enp1s0f1") # <--- put here list of network cards you have (devices names) they will be in order your nicnr 1,2,3...
# ---

# // -------------------------------------------------------------------

ipTARGET=( "$@" )


echo "---------------------"
echo "Your cards (configured in $config_global)"
ix=1
for target in "${ipCARD[@]}"
do
	dev=${ipCARD[ix-1]} # -1 to move to 0-based bash array indexes
	echo "Your cardnr :$ix is $dev"
	ix=$((ix+1))
	ip address flush dev ${dev}
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

conn_ix=0
for target in "${ipTARGET[@]}"
do
	echo "Parsing connection command [$target]"
	target2=$( echo "$target" | sed -e's/:/ /g')
	read my_nicnr target_base target_nicnr <<< "$target2" || fail "Parse error in $[target]"
	[[ "$my_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target] ($my_nicnr) my_nicnr"
	[[ "$target_base" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target] ($target_base) target_base"
	[[ "$target_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target]  ($target_nicnr) target_nicnr"

	[[ "$my_nicnr" -gt 9 ]] && fail "Too big my_nicnr ($my_nicnr)"
	[[ "$target_base" -gt 9 ]] && fail "Too big target_base ($target_base)"
	[[ "$target_nicnr" -gt 9 ]] && fail "Too big target_nicnr ($target_nicnr)"

	# b1 and b2: if I am base 4, and target is 7 then our common net is .74. sorted, so .47.
	# this assumes that b1<b2, then build net b1.b2 and subnet c1.c2
	b1="$ipMyBase" ; b2="$target_base"
	n1="$my_nicnr" ; n2="$target_nicnr"
	[[ "$b1" -gt "$b2" ]] && { # but if b1>b2 then reverse it:
		tmp="$b1" ; b1="$b2" ; b2="$tmp";
		tmp="$n1" ; n1="$n2" ; n2="$tmp";
	}
	[[ "$b1" == "$b2" ]] && fail "Trying to connect to yourself? You are base ipMyBase=$ipMyBase , target base = $target_base"
	baseNet="$b1$b2"
	nicSubnet="$n1$n2"

	my_endip=$(( ipMyBase*10 + my_nicnr ))
	target_endip=$(( target_base*10 + target_nicnr ))

	my_ip="${ipBlock}.${baseNet}.${nicSubnet}.${my_endip}"
	target_ip="${ipBlock}.${baseNet}.${nicSubnet}.${target_endip}"

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

	conn_ix=$(( conn_ix+1 ))
	echo "${target_ip}" > "${ipVarDir}/ip-nic-${my_nicnr}.txt"
	echo "${target_ip}" > "${ipVarDir}/ip-conn-${conn_ix}.txt"
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


