// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"

void c_galaxysrv_peers::add_peer(const string & refer) {
	_info("Add peer: " << refer);
}

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"--peer 192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer auto:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer udp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer tcp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer shm:test1-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]
	;
}

