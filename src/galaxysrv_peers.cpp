// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"
#include "libs1.hpp"

t_peering_reference_parse c_galaxysrv_peers::parse_peer_reference(const string & simple) {
	// "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"
	size_t pos1 = simple.find('-');
	if (pos1 == string::npos) { // must be one-part format [cable]
		vector<string> ret_id; // no IDs
		vector<string> ret_cable( simple.substr(pos1+1) );
		return std::make_pair(ret_id, ret_cable);
	}
	else { // other format
		_try_user(pos1>0); // string::substr is safe, but anyway test against user doing --peer "-foo"
		string part1 = simple.substr(0,pos1);
		string part2 = simple.substr(pos1+1);

		if (part1=="id") { // format [id] or format [id-cables]
			if (part2=="") { // format [id]
			}
			else { // format [id-cables]
			}
		}

		try {
			/*
			_info("Peer pip="<<part_pip<<" hip="<<part_hip);
			auto ip_pair = tunserver_utils::parse_ip_string(part_pip);
			_note("Physical IP: address=" << ip_pair.first << " port=" << ip_pair.second);
			// this->add_peer_simplestring( t_peering_reference( ip_pair.first, ip_pair.second , part_hip ) );
			*/
		}
		catch (const std::exception &e) {
			_erro(mo_file_reader::gettext("L_failed_adding_peer_simple_reference") << e.what());
			_throw_error( std::invalid_argument(mo_file_reader::gettext("L_bad_peer_format")) );
		}
	} // other format
}

t_peering_reference_parse c_galaxysrv_peers::add_peer_simplestring(const string & simple) {
	_info("Adding peer from simplestring=" << simple);
	t_peering_reference_parse parse = parse_peer_reference(simple);
}

/*

Cable types (TODO move this doc later to cable base addr)

Cable types are:
"udp4" "udp6" "tcp4" "tcp6" "ETH"

When taking input from user (e.g. in the factory generating addresses) we also support
this conversions:

"" --> "auto"
"auto" --> "udp"
"udp" --> udp4 or udp6, auto detect from address form (x.x.x.x)
"tcp" --> udp4 or udp6, auto detect from address form (x.x.x.x)

Btw missing port at end is defaulted to 9042.

So that "auto:4.5.6.7" and "4.5.6.7" and "udp:4.5.6.7" and "udp4:4.5.6.7:9042"
will all be expanded to address UDPv4 4.5.6.7 port 9042, printed as normalized "udp4:4.5.6.7:9042"

*/

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"# Format [cable] - peer to anyone who is reachable there:\n"
	"--peer 192.166.218.58 \n"
	"--peer auto:192.166.218.58 \n"
	"--peer auto:192.166.218.58:9042 \n"
	"--peer udp:192.166.218.58:9042 \n"
	"--peer tcp:192.166.218.58:9042 \n"
	"\n"
	"# Format [id] - peer to this ID, find yourself how (e.g. using seed-nodes):\n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5\n"
	"\n"
	"# Format [id-cables] - peer to this ID, using given list of cables:\n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(tcp:192.166.218.58:9042) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(tcp:192.166.218.58:9042,udp:192.166.218.58:9042) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(shm:test1,shm:test2) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(shm:test1,shm:test2,tcp:192.166.218.58:9042) \n"
	"\n"
	"# Format [cable-id] - (an older format):\n"
	"--peer 192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer 192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer auto:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer udp:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer tcp:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer shm:test1-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	;
}

