
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

valgrind  --tool=helgrind  --num-callers=100  -- build/nocap-tunserver.elf --newloop --peer "fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107:9042)"  "$@"


