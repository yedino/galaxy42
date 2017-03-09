// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"
#include "libs1.hpp"

c_galaxysrv_peers::t_peering_reference_parse c_galaxysrv_peers::parse_peer_reference(const string & simple) const{
	// @TODO not using std::regex since it had compatibility problems. Consider using when possible (bug#J446).
	const char separator='@', group_open='(', group_close=')';

	_mark("Parsing: "<<simple);
	size_t pos1 = simple.find(separator);

	if (pos1 == string::npos) { // must be one-part format "VIRTUAL"
		vector<string> ret_id( { simple } ); // e.g. "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,score=-300"
		vector<string> ret_cable{ };
		return std::make_pair(std::move(ret_id), std::move(ret_cable));
	}
	else { // format "VIRTUAL@(CABLE)@(CABLE)"
		string part1 = simple.substr(0,pos1); // the VIRTUAL
		vector<string> ret_id( { part1 } ); // e.g. "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,score=-300"
		// fd42::25f5@(udp:p.meshnet.pl:9042,cost=500)@(shm:test)@(tcp:[fe80::d44e]:9042)
		//             B
		//             B.............................X
		//                                           X@(B
		//                                              B.......X@
		//                                                      X@(B
		//                                                         B.....................end

		vector<string> ret_cable;
		// will parse additional groups "@(....)" selecting their index posB...posX
		const size_t size = simple.size(), size_before=size-1; _check(size_before < size);
		size_t posB = pos1+2; // begin, poiting after "@("
		_info(join_string_sep(posB," < ",size));
		while (posB < size_before) {
			auto posX = simple.find(group_close,posB); // find ")"
			_info("posX=" << posX << " posB="<<posB);
			if (posX == string::npos) {
				_info("Hit end because posX="<<posX);
				posX = size_before;
			}
			string partX = simple.substr(posB, posX-posB);
			_info("posX=" << posX << " posB="<<posB<<" given " <<partX);
			ret_cable.push_back( std::move(partX) );
			posB=posX;
			_note(simple.at(posB));
			_check_user(simple.at(posB)==group_close); // ")"
			++posB;
			if (!(posB<size)) break; // end is possible after last "....@(...)"
			// otherwise we open new group:
			_check_user(simple.at(posB)==separator); // ")@"
			++posB;
			_check_user(simple.at(posB)==group_open); // ")@(" after this
			++posB;
		}

		return std::make_pair(std::move(ret_id), std::move(ret_cable));

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
	bool id_anyone=true;
	string id;
	const auto & cables = parse.second;
	if (parse.first.size()==1) { // there was 1 ID parsed
		id_anyone=false;
		id=parse.first.at(0);
	} else _check(parse.first.size() == 0); // else there was no ID parsed
	_note(join_string_sep( id_anyone?"anyone":"id" , id ) );
	_note("Cables: " << cables.size());
	for(const auto & cablestr : cables) _note("Cable: " << cablestr);
}

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"# --peer VIRTUAL \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,score=-300 \n"
	"# --peer VIRTUAL@(CABLE) \n"
	"  --peer anyone@(udp:76.31.171.15:9042) \n"
	"  --peer anyone@(udp:p.meshnet.pl:9042) \n"
	"  --peer anyone@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:76.31.171.15:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p2.meshnet.pl:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:76.31.171.15:9042,cost=100) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend@(udp:76.31.171.15:9042) \n"
	"# --peer VIRTUAL@(CABLE)@(CABLE) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10@(udp:76.31.171.15:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p.meshnet.pl:9042,cost=500)@(shm:test)@(tcp:[fe80::d44e]:9042) \n"
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

