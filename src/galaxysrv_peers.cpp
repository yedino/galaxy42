// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"
#include "libs1.hpp"

c_galaxysrv_peers::t_peering_reference_parse c_galaxysrv_peers::parse_peer_reference(const string & simple) const{
	// "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"
	_info("Parsing: "<<simple);
	size_t pos1 = simple.find('-');

/*
*/

	if (pos1 == string::npos) { // must be one-part format [CABLE]
		vector<string> ret_id; // no IDs
		vector<string> ret_cable{ simple.substr(pos1+1) };
		return std::make_pair(std::move(ret_id), std::move(ret_cable));
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

	t_peering_reference_parse x;
	return x;
}

void c_galaxysrv_peers::add_peer_simplestring(const string & simple) {
	_info("Adding peer from simplestring=" << simple);
	t_peering_reference_parse parse = parse_peer_reference(simple);
}

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"# Format [id-ID-CABLES] - peer to this ID, using given list of cables:\n"
	"--peer udp:76.31.171.15:9042 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-udp:76.31.171.15:9042 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-udp:76.31.171.15:9042,cost=100 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend-udp:76.31.171.15:9042 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10-udp:76.31.171.15:9042 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-udp:p2.meshnet.pl:9042 \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(udp:p.meshnet.pl:9042,cost=500),(shm:test,tcp:[fe80::d44e]:9042) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr \n"
	"--peer bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999 \n"
	"--peer (bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999;udp:p.meshnet.pl:9042) \n"
	"--peer (udp:p.meshnet.pl:9042;shm:test;tcp:[fe80::d44e]:9042) \n"
	"\n"
	"CABLE is: \n"
	"cabletype:data:port \n"
	"cabletype:data:port \n"
	"cabletype:data:port"
	"a cable, with possible types are: \n"
	"'udp4' 'udp6' 'tcp4' 'tcp6' 'ETH' (currently implemented: ...TODO) \n" // TODO-release update what is implemented
	"\n"
	"When taking input from user (e.g. in the factory generating addresses) we also support \n"
	"this conversions: \n"
	" \n"
	"\"\" --> \"auto\" \n"
	"\"auto\" --> \"udp\" \n"
	"\"udp\" --> udp4 or udp6, auto detect from address form (x.x.x.x) \n"
	"\"tcp\" --> udp4 or udp6, auto detect from address form (x.x.x.x) \n"
	" \n"
	"Btw missing port at end is defaulted to 9042. \n"
	" \n"
	"So that \"auto:4.5.6.7\" and \"4.5.6.7\" and \"udp:4.5.6.7\" and \"udp4:4.5.6.7:9042\" \n"
	"will all be expanded to address UDPv4 4.5.6.7 port 9042, printed as normalized \"udp4:4.5.6.7:9042\" \n"
	" \n"
;
}

