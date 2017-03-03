// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"

void c_galaxysrv_peers::add_peer_simplestring(const string & simple) {
	_info("Adding peer from simplestring=" << simple);

	// "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"
	size_t pos1 = simple.find('-');
	if (pos1 == string::npos) { // must be one-part format "--peer auto:192.166.218.58:9042 \n"
	}
	else {
		string part1 = simple.substr(0,pos1);
		string part2 = simple.substr(pos1+1);
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
		}
}

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"# peer to anyone who is reachable there:\n"
	"--peer 192.166.218.58 \n"
	"--peer auto:192.166.218.58 \n"
	"--peer auto:192.166.218.58:9042 \n"
	"--peer udp:192.166.218.58:9042 \n"
	"--peer tcp:192.166.218.58:9042 \n"
	"\n"
	"# peer to this ID, find yourself how (e.g. using seed-nodes):\n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5\n"
	"\n"
	"# peer to this ID, using given list of cables:\n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(tcp:192.166.218.58:9042) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(tcp:192.166.218.58:9042,udp:192.166.218.58:9042) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(shm:test1,shm:test2) \n"
	"--peer id-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5-(shm:test1,shm:test2,tcp:192.166.218.58:9042) \n"
	"\n"
	"# cable+ID (older format):\n"
	"--peer 192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer 192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer auto:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer udp:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer tcp:192.166.218.58:9042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	"--peer shm:test1-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 \n"
	;
}

