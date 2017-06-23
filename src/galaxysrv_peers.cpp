// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "galaxysrv_peers.hpp"
#include "libs1.hpp"

void t_peer_reference_newloop::print(ostream &ostr) const{
	ostr << hip << ", cables:[";
	for(const unique_ptr<c_cable_base_addr>& addr : cable_addr)
		ostr << UsePtr(addr) << ", ";
	ostr << "\b\b], options: {";
	for(const std::pair<string, boost::any>& opt : options)
	{
		ostr << opt.first << ':';
		try{
			ostr << boost::any_cast<int>(opt.second) << ", ";
		}catch(const boost::bad_any_cast &)
		{
			try{
				ostr << boost::any_cast<string>(opt.second) << ", ";
			}catch(const boost::bad_any_cast &)
			{
				ostr << "?, ";
			}
		}
	}
	if(options.size())
		ostr << "\b\b";
	ostr << '}' << endl;
}

ostream& operator<<(ostream &ostr, const t_peer_reference_newloop & v){
	v.print(ostr);
	return ostr;
}

c_peer_connection::c_peer_connection( t_peer_reference_newloop && ref )
 : m_reference( std::move(ref) )
{
}

bool c_peer_connection::is_connected() const {
	// TODO
	return true;
}

bool c_peer_connection::should_connect() const {
	// TODO
	return true;
}

void c_peer_connection::print(ostream &ostr) const {
	ostr << m_reference;
}

ostream& operator<<(ostream &ostr, const c_peer_connection & v){
	v.print(ostr);
	return ostr;
}

c_galaxysrv_peers::t_peering_reference_parse c_galaxysrv_peers::parse_peer_reference(const string & simple) const{
	// @TODO not using std::regex since it had compatibility problems. Consider using when possible (bug#J446).
	const char separator='@', group_open='(', group_close=')';
	const string literal_anyone = "anyone";
	reasonable_size(simple);

	_note("Parsing: "<<simple);
	_check_input(simple.size()>=3); // some reasonable sized not-empty string

	size_t pos1 = simple.find(separator); // was there any "@"

	if (pos1 == string::npos) { // must be one-part format "VIRTUAL" (only contains the ID/ipv6, no cables)
		vector<string> ret_id( { simple } ); // e.g. "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,score=-300"
		vector<string> ret_cable{ };
		// make sure this part does not contain ( or ) etc
		_check_input( string::npos == simple.find(group_open) );
		_check_input( string::npos == simple.find(group_close) );
		_check_input( simple.size()>0 );
		return std::make_pair(std::move(ret_id), std::move(ret_cable));
	}
	else { // format "VIRTUAL@(CABLE)" possibly with more "@(CABLE)"
		string part1 = simple.substr(0,pos1); // the VIRTUAL
		// make sure this part does not contain ( or )
		_check_input( string::npos == part1.find(group_open) );
		_check_input( string::npos == part1.find(group_close) );
		_check_input( part1.size()>0 );
		vector<string> ret_id;
		if (part1 != literal_anyone) {
			ret_id.push_back( part1 ); // e.g. "fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,score=-300"
		} // else, it is 'anyone' VIRTUAL - so it is reported by empty string

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
		_check_input(pos1+2 < size);
		_check_input(simple.at(pos1+0)==separator);  // theck the '@' out of "@(" that we just found
		_check_input(simple.at(pos1+1)==group_open); // theck the '(' out of "@(" that we just found
		size_t posB = pos1+2; // begin, poiting after "@("
		_info(join_string_sep(posB," < ",size));
		while (posB < size_before) {
			auto posX = simple.find(group_close,posB); // find ")"
			_dbg3("posX=" << posX << " posB="<<posB);
			if (posX == string::npos) {
				_info("Hit end because posX="<<posX);
				posX = size_before;
			}
			string partX = simple.substr(posB, posX-posB);
			_dbg3("posX=" << posX << " posB="<<posB<<" given " <<partX);
			// make sure this part does not contain ( or ), e.g. "CABLE(" as result of parsing "VIRTUAL@(CABLE(CABLE)"
			_check_input( string::npos == partX.find(group_open) );
			_check_input( string::npos == partX.find(group_close) );
			_check_input( partX.size()>0 );

			ret_cable.push_back( std::move(partX) );
			posB=posX;
			_check_input(simple.at(posB)==group_close); // ")"
			++posB;
			if (!(posB<size)) break; // end is possible after last "....@(...)"
			// otherwise we open new group:
			_check_input(simple.at(posB)==separator); // ")@"
			++posB;
			_check_input(simple.at(posB)==group_open); // ")@(" after this
			++posB;
		}
		return std::make_pair(std::move(ret_id), std::move(ret_cable));
	} // other format
}

void c_galaxysrv_peers::add_peer(unique_ptr<t_peer_reference_newloop> && ref) {
	_check(ref.get());
	_note("Adding peer: "); // TODO << *ref );

	auto peer = make_unique<c_peer_connection>( std::move( *ref ) );
	m_peer.push_back( std::move( peer ) );
}

void c_galaxysrv_peers::add_peer_simplestring(const string & simple) {
	_clue("Adding peer from simplestring=" << simple);
	t_peering_reference_parse parse = parse_peer_reference(simple); // partially parsed
	_dbg1("Done the parse itself");
	bool id_anyone=true;
	string id;
	const auto & cables = parse.second;

	auto reference = make_unique<t_peer_reference_newloop>();

	if (parse.first.size()==1) { // there was 1 ID parsed
		_dbg1("There is some ID in that reference.");
		id_anyone=false;
		id = parse.first.at(0);
		reference->hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , id);
	} else {
		_dbg1("There is NO ID in that reference (anyone?)");
		id_anyone=true;
		_check(parse.first.size() == 0); // else there was no ID parsed
		reference->hip = c_haship_addr::make_empty(); // clear it to be sure
	}
	_note(join_string_sep( id_anyone?"anyone":"id" , id ) );
	_note("Cables: " << cables.size());
	for(const auto & cablestr : cables) {
		_note("Cable: " << cablestr); // cablestr like udp:192.168.1.107:9042
		unique_ptr<c_cable_base_addr> cable_addr = c_cable_base_addr::cable_make_addr( cablestr );
		reference->cable_addr.push_back( std::move(cable_addr) );
	}
	this->add_peer( std::move( reference ) ); // ***
}

void c_galaxysrv_peers::help_peer_ref(ostream & ostr) {
	ostr <<
	"# --peer VIRTUAL \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=-300 \n"
	"# --peer VIRTUAL@(CABLE) \n"
	"  --peer fd42::1234@(udp:foo.com:9042)@(email:tx@test.com)@(email:b@test.com)@(email:c@test.com) \n"
	"  --peer anyone@(udp:76.31.171.15:9042) \n"
	"  --peer anyone@(udp:p.meshnet.pl:9042) \n"
	"  --peer anyone@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr,cost=99999) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:76.31.171.15:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p2.meshnet.pl:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:76.31.171.15:9042,cost=100) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(bitmsg:BM-NBqsBxsE1F1pxAgKpMesHFhTy6UYbcFr) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend@(udp:76.31.171.15:9042) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5,friend,score=10000,prio=10@(udp:76.31.171.15:9042) \n"
	"# --peer VIRTUAL@(CABLE)@(CABLE) \n"
	"  --peer fd42:f6c4:9d19:f128:30df:b289:aef0:25f5@(udp:p.meshnet.pl:9042,cost=500)@(shm:test)@(tcp:[fe80::d44e]:9042) \n"
	"\n"
	"CABLE is: \n"
	"cabletype:data:port \n"
	"cabletype:data:port \n"
	"cabletype:data:port"
	"a cable, with possible types are: \n"
	"'udp4' 'udp6' 'tcp4' 'tcp6' 'ETH' (currently implemented: ...TODO) \n" // TODO-release update what is implemented
	"@@@@@ WARNING: only 'udp' type is implemented currently! @@@@@"
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

