#!/bin/bash
# iplab2 - Copyrighted (C) 2018 rfree ; BSD 2-clause licence
# Warning: Quick code for tests, might be insecure !!!
# This configures IPv4 numbers based on p2p9x9 configuration.
# See help_usage() or run --help for information and purpose of this script,
# there is a link to the wiki too.

# set -o errexit
# set -o nounset

function fail() { echo "Error: $*" ; exit 1; }

ipConfigFile='/etc/iplab2/global.sh'

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
	echo "Usage:"
	echo '(0)'
	echo "  program -c # creates the default configuration"
	echo "  program -h or --help # shows help"
	echo '(1) direct input of connections from this computer:'
	echo '  program -w "1-3:1,2-4:1,3-4:9"'
	echo '  program -w "connect1,connect2,connect3..."'
	echo '  connect is: "my_nic-target_base:target_nic"'
	echo '  connect "2-4:9" means: my 2-nd card, goes to 4-th computer into his 9-th card'
	echo '  connect "3-4:8" means: my 3-rd card, goes to 4-th computer into his 8-th card'
	echo "    my_nic and target_nic - the card numbers, on each computer, are numbered from 1 (so 1,2,3...) and order depends given computer's configuration ipCARD variable. Allowed range is 1..9."
	echo "    target_base is the base-number of given computer (as he configured in his configuration ipMyBase variable). Allowed range is 1..9/"
	echo '(2) provide file with network_layout'
	echo '  program -f "blitz4.txt"  # will read from file like "/etc/iplab2/p2p9x9/layout/blitz4.txt" depending on config'
	echo '(3) provide raw URL of network_layout'
	echo '  program -U "http://[fd42:...]/p2p9x9/blitz4.txt"'
	echo '(3b) provide URL filename (appended at end of URL base from config, also suffix .txt is added) of network_layout.'
	echo '  program -u "blitz4"'
	echo "More information on wiki: https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9"
	echo ""
}
[[ -z "$*" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "-h" ]] && { help_usage ; exit 40 ; }
[[ "$1" == "--help" ]] && { help_usage ; exit 40 ; }

function my_create_config() {
	mkdir -p '/etc/iplab2/'
	mkdir -p '/etc/iplab2/p2p9x9/layout/'
	if [[ -r "$ipConfigFile" ]] ; then
		echo "Main config already existed"
	else
		cat << 'EOFEOFEOF' > "$ipConfigFile"
#!/bin/bash
readonly ipBlock="10"
readonly ipNetmask="24"
readonly ipVarDir="/var/local/iplab2/"
readonly ipFileBase="/etc/iplab2/p2p9x9/layout/" # usually leave it as is. for "-f" option to read configuration from.
# my network setup
readonly ipUrlBase="http://[fd42:....]/p2p9x9/" # or leave this empty. The url for "-u" option to download configuration from.
# this computer - you MUST configure this:
readonly ipMyBase=0 # <--- put here base-number of current computer
declare -a ipCARD=("enp1s0f0" "enp1s0f1") # <--- put here list of network cards you have (devices names) they will be in order your nicnr 1,2,3..., e.g. "enp1s0f0" "enp1s0f1"
EOFEOFEOF
	fi

	f='/etc/iplab2/p2p9x9/layout/pair.txt'
	if [[ -r "$f" ]] ; then
		echo "Config ($f) already exists"
	else
		cat << 'EOFEOFEOF' > "$f"
# p2p9x9 ip config for iplab2 - see https://github.com/yedinocommunity/galaxy42/wiki/p2p9x9
# Just two computers, each with one card, connected
1;1-2:1
# ^- for computer1: take your card1, and connect to computer2 his card1
2;1-1:2
# ^- for computer2: take your card1, and connect to computer1 his card1
EOFEOFEOF

echo "Config creation done (please edit it: $ipConfigFile)"

	fi
}

[[ "$1" == "-c" ]] && { my_create_config ; exit 40 ; }

# // -------------------------------------------------------------------

source "$ipConfigFile" || fail "Failed to run global config script [$ipConfigFile] - read my source code for instructions how to create it; Run me with --help to read instructions."


# // -------------------------------------------------------------------

# ipTARGET=( "$@" )

function apply_my_wires() {

IFS=',' read -r -a ipTARGET <<< "$1"


echo "---------------------"
echo "Your cards (configured in $ipConfigFile)"
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
	echo "${target_ip}" > "${ipVarDir}/nic${my_nicnr}-dstip.txt"
	echo "${target_ip}" > "${ipVarDir}/conn${conn_ix}-dstip.txt"
	echo "${my_ip}" > "${ipVarDir}/nic${my_nicnr}-srcip.txt"
	echo "${my_ip}" > "${ipVarDir}/conn${conn_ix}-srcip.txt"
	echo "${my_eth}" > "${ipVarDir}/nic${my_nicnr}-dev.txt"
	echo "${my_eth}" > "${ipVarDir}/conn${conn_ix}-dev.txt"
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
echo "You can now run commands that pick up the test IPs (and dev name) like this:"
echo 'ping "$(< /var/local/iplab2/nic1-dstip.txt)"   # using 1st of your NIC cards as configured nicnr in your settings'
echo 'ping "$(< /var/local/iplab2/conn1-dstip.txt)"   # 1st connection from above - IP (it always exists if you connected anything)'
echo 'ping "$(< /var/local/iplab2/conn2-dstip.txt)"   # 2nd connection from above (if you have it) - IP'
echo "etc..."

echo "Done - applied my wires ($1)"

}

function apply_network_layout() {
	the_source="$1"
	the_url="$2"
	if [[ "$the_source" == "curl" ]] ; then
		echo "Loading from network ($the_url)"
	elif [[ "$the_source" == "file" ]] ; then
		echo "Loading from file ($the_url)"
	else
		fail "Invalid source ($the_source)"
	fi

	the_wire=''
	while IFS= read -r line; do
		IFS=';' read -r computer wire <<< "$line"
		echo "Read, computer $computer (looking for $ipMyBase) wire $wire"
		if [[ "$computer" == "$ipMyBase" ]] ; then
			echo "OK, Found a config for me ($wire)"
			the_wire="$wire"
			break
		fi
	done < <(
		if [[ "$the_source" == "curl" ]] ; then
			curl -- "$the_url" | egrep -v '^#.*'
		elif [[ "$the_source" == "file" ]] ; then
			cat "$the_url" | egrep -v '^#.*'
		fi
	)

	if [[ -n "$the_wire" ]] ; then
		apply_my_wires "$the_wire"
	else
		echo "No wires configured for me, for computer number $ipMyBase."
	fi
}

function apply_url_raw() {
	read_network_layout
}


# main script:

# ipUrlBase="http://.../"

if [[ "$1" == "-w" ]] ; then
	apply_my_wires "$2"
elif [[ "$1" == "-f" ]] ; then
	# relative file name
	[[ -z "$ipFileBase" ]] && fail "Please configure ipFileBase option in configuration (it is empty now) to use this option."
	param="$2"
	[[ "$param" =~ ^[0-9a-zA-Z_-]+$ ]] || fail "Invalid file name provided [$param] (insecure, but be simple alphanum_- name, no dirs, no dots, we will add .txt)"
	apply_network_layout "file" "${ipFileBase}${param}.txt"
elif [[ "$1" == "-u" ]] ; then
	# relative URL name
	[[ -z "$ipUrlBase" ]] && fail "Please configure ipUrlBase option in configuration (it is empty now) to use this option."
	param="$2"
	[[ "$param" =~ ^[0-9a-zA-Z_-]+$ ]] || fail "Invalid file name provided [$param] (insecure, but be simple alphanum_- name, no dirs, no dots, we will add .txt)"
	apply_network_layout "curl" "${ipUrlBase}${2}.txt"
elif [[ "$1" == "-U" ]] ; then
	# full URL, must start with http:// or https://
	param="$2"
	[[ "$param" =~ ^(http://|https://).+$ ]] || fail "Invalid file name provided [$param] (must be a full URL starting with http:// or https://)"
	apply_network_layout "curl" "${2}"
else
	echo "Unknown command ($1). Use --help to see help."
	exit 1
fi


