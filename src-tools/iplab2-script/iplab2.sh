#!/bin/bash
# iplab2 - Copyrighted (C) 2018 rfree ; BSD 2-clause licence
# Warning: Quick code for tests, might be insecure !!!
# This configures IPv4 numbers based on p2p9x9 configuration.
# See help_usage() or run --help for information and purpose of this script,
# there is a link to the wiki too.

# set -o errexit
# set -o nounset

function fail() { echo "Error: $*" ; exit 1; }

function help_usage() {
	echo ""
	echo "Script to assign IPv4 addresses in format 'p2p9x9' - where up to 9 computers each with up to 9 cards are connected in pairs of poin-to-point connections."
	echo "Choose a base number 1..9 for each of your computers. Then install (configure) this script on each computer"
	echo "When you run this script on each computer, telling it the connections from given one computer to others, then all cards on all computers end up with properly configured IPs and networks"
	echo ""
	echo "Script assigns IPs to cards. All cards are point-to-point connections from one computer to another."
	echo "When cable connects computer A, card X -to- computer B, card Y - then:"
	echo "if A<B then computer A gets IP 10.AB.XY.AX, computer B gets 10.AB.XY.BY"
	echo "if A>B then computer A gets IP 10.BA.YX.AX, computer B gets 10.BA.YX.BY"
	echo "A is always not equal to B, since computer base numbers are unique. X and Y could be the same (they are unique only between cards of given computer)."
	echo "A,B,X,Y are 1..9. IPs like 10.1.*.* up to 10.11.*.*, and above 10.99.*.*, will never be configured by p2p9x9, leaving it free eg to setup VPN too"
	echo ""
	echo "For example: computers: Alice(6) Bob(7)"
	echo "Alice(6):1 --- Bob(7):2 (Computer alice base number 6 connects her card 1 into computer Bob base number 7 his card 2)"
	echo "Alice(6):1 --- Bob(7):2 will get IPs: 10.67.12.61 --- 10.67.12.72"
	echo "Alice(6):2 --- Bob(7):1 will get IPs: 10.67.21.62 --- 10.67.21.71"
	echo "Script assigns IP by following rules:"
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
	echo '(1) direct input of connections from this computer:'
	echo '  program -w "1-3:1,2-4:1,3-4:9"'
	echo '  program -w "connect1,connect2,connect3..."'
	echo '  connect is: "my_nic-target_base:target_nic"'
	echo '  connect "2-4:9" means: my 2-nd card, goes to 4-th computer into his 9-th card'
	echo '  connect "3-4:8" means: my 3-rd card, goes to 4-th computer into his 8-th card'
	echo "    my_nic and target_nic - the card numbers, on each computer, are numbered from 1 (so 1,2,3...) and order depends given computer's configuration ipCARD variable. Allowed range is 1..9."
	echo "    target_base is the base-number of given computer (as he configured in his configuration ipMyBase variable). Allowed range is 1..9/"
	echo '(2) [TODO] provide file with network-configuration'
	echo '  program -F "/etc/iplab2/p2p9x9/blitz4.txt"'
	echo '(3) provide raw URL of network-configuration'
	echo '  program -U "http://[fd42:...]/p2p9x9/blitz4.txt"'
	echo '(3b) [TODO] provide end of URL of network-configuration. Base of the URL is read from configuration file (see script source) and ".txt" is appended.'
	echo '  program -u "blitz4"'
	echo "program -h or --help shows help"
	echo "More information on wiki: https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9"
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
# # my network setup
# readonly ipUrlBase="http://[fd42:....]/p2p9x9/" # or leave this empty. The url for "-u" option to download configuration from
# # this computer:
# readonly ipMyBase=4 # <--- put here base-number of current computer
# declare -a ipCARD=("enp1s0f0" "enp1s0f1") # <--- put here list of network cards you have (devices names) they will be in order your nicnr 1,2,3...
# ---

# // -------------------------------------------------------------------

# ipTARGET=( "$@" )

function apply_my_wires() {

IFS=',' read -r -a ipTARGET <<< "$1"


echo "---------------------"
echo "Your cards (configured in $config_global)"
ix=1
for target in "${ipCARD[@]}"
do
	dev=${ipCARD[ix-1]} # -1 to move to 0-based bash array indexes
	echo "Your cardnr :$ix is $dev"
	ix=$((ix+1))
	ip address flush dev "${dev}" # action
done
echo "---------------------"
echo

[[ "$ipMyBase" -gt 9 ]] && fail "Too big ipMyBase ($ipMyBase)"
[[ "$ipMyBase" -lt 1 ]] && fail "Too small ipMyBase ($ipMyBase)"
[[ -z "$ipVarDir" ]] && fail "Configure ipVarDir."
[[ "$ipVarDir" =~ ^/[a-zA-Z0-9/]{1,100}/iplab2/$ ]] || fail "Invalid configuration ipVarDir (regex)"

[[ -w "${ipVarDir}" ]] || {
	mkdir -p "${ipVarDir}" || fail "Can not create dir $ipVarDir"
}

for ifile in "${ipVarDir}"/*
do
	[[ -e "$ifile" ]] || break
	rm --preserve-root "${ifile}"
done
chmod a+rX "${ipVarDir}/"

conn_ix=0
for target in "${ipTARGET[@]}"
do
	echo "Parsing connection command [$target]"
	target2=$( echo "$target" | sed -e's/:/ /g' | sed -e's/-/ /g') # change "1-2:3" into "1 2 3"
	read my_nicnr target_base target_nicnr <<< "$target2" || fail "Parse error in $[target]" # read "1 2 3"

	[[ "$my_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target] ($my_nicnr) my_nicnr"
	[[ "$target_base" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target] ($target_base) target_base"
	[[ "$target_nicnr" =~ ^[0-9]{1,3}$ ]] || fail "Not a valid integer in [$target]  ($target_nicnr) target_nicnr"

	[[ "$my_nicnr" -gt 9 ]] && fail "Too big my_nicnr ($my_nicnr)"
	[[ "$my_nicnr" -lt 1 ]] && fail "Too small my_nicnr ($my_nicnr)"
	[[ "$target_base" -gt 9 ]] && fail "Too big target_base ($target_base)"
	[[ "$target_base" -lt 1 ]] && fail "Too small target_base ($target_base)"
	[[ "$target_nicnr" -gt 9 ]] && fail "Too big target_nicnr ($target_nicnr)"
	[[ "$target_nicnr" -lt 1 ]] && fail "Too small target_nicnr ($target_nicnr)"

	# b1 and b2: if I am base 4, and target is 7 then our common net is .74. sorted, so .47.
	# this assumes that b1<b2, then build net b1.b2 and subnet n1.n2
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
	ip address flush dev "${my_eth}" # action
	ip link set dev "${my_eth}" up # action
	ip addr add "${my_ip}/${ipNetmask}" dev "$my_eth" # action
	ifconfig "${my_eth}" mtu 9000 # action
	#set +x

	ping -w 1 -W 1 -c 1 "$target_ip" > /dev/null && echo "^--- PING OK ($target_ip)"
	echo ""

	conn_ix=$(( conn_ix+1 ))
	echo "${target_ip}" > "${ipVarDir}/ip-nic-${my_nicnr}.txt"
	echo "${target_ip}" > "${ipVarDir}/ip-conn-${conn_ix}.txt"
	echo "${my_eth}" > "${ipVarDir}/dev-nic-${my_nicnr}.txt"
	echo "${my_eth}" > "${ipVarDir}/dev-conn-${conn_ix}.txt"
done

echo ""
echo "---------------------"
echo "See IPs in files in $ipVarDir (use in scripts etc)"
for ifile in "${ipVarDir}"*.txt
do
	[[ -e "$ifile" ]] || break
	echo -n "${ifile}: "
	cat "${ifile}"
done

echo
echo "You can now run commands that pick up the test IPs like this:"
echo 'ping "$(< /var/local/iplab2/ip-conn-1.txt)"   # 1st connection from above'
echo 'ping "$(< /var/local/iplab2/ip-conn-2.txt)"   # 2nd connection from above (if you have it)'
echo 'ping "$(< /var/local/iplab2/ip-nic-1.txt)"   # using 1st of your NIC cards as configured here, no matter what connection'
echo "etc..."

echo "Done - applied my wires ($1)"

}

function apply_url_raw() {
	the_url="$1"
	the_wire=""
	echo "Downloading and parsing file from ($the_url)"
	while IFS= read -r line; do
		IFS=';' read -r computer wire <<< "$line"
		echo "Read, computer $computer (looking for $ipMyBase) wire $wire"
		if [[ "$computer" == "$ipMyBase" ]] ; then
			echo "OK, Found a config for me ($wire)"
			the_wire="$wire"
			break
		fi
	done < <(curl -- "$the_url" | egrep -v '^#.*')

	if [[ -n "$the_wire" ]] ; then
		apply_my_wires "$the_wire"
	else
		echo "No wires configured for me, for computer number $ipMyBase."
	fi
}


# main script:

# ipUrlBase="http://.../"

if [[ "$1" == "-w" ]] ; then
	apply_my_wires "$2"
elif [[ "$1" == "-F" ]] ; then
	echo "Not implemented"
	exit 2
elif [[ "$1" == "-u" ]] ; then
	apply_url_raw "${ipUrlBase}${2}.txt"
elif [[ "$1" == "-U" ]] ; then
	apply_url_raw "$2"
else
	echo "Unknown command ($1). Use --help to see help."
	exit 1
fi





