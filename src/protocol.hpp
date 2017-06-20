// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

// ------------------------------------------------------------------

class c_protocolv3 {
	public:
		constexpr static unsigned char current_version = 2;

		constexpr static unsigned char version_size = 0; // merged with command
		constexpr static unsigned char cmd_size = 1;

		/// max expected size of header of p2p-data. (for logical data, transport headers are something else)
		constexpr static unsigned char max_header_size = 128;

		enum class t_proto_cmd : unsigned char {
			e_proto_cmd_unused_old_version_0 = 0, // old version 0
			e_proto_cmd_unused_old_version_1 = 1, // old version 1
			e_proto_cmd_unused_old_version_2 = 2, // old version 2 (unused in old program?)
			e_proto_cmd_data_one_merit_clear = 3, // data: just 1 merit (no bags/welds). No crypto (encrypt/auth).
			e_proto_cmd_data_cart = 4, // data in the new format of a cart
			//e_proto_cmd_public_hi = 3, // simple public peering
			//e_proto_cmd_public_ping_request = 4, // simple public ping to the peer
			//e_proto_cmd_public_ping_reply = 5, // simple public ping to the peer
			//e_proto_cmd_findhip_query = 10, // searching HIP - query
			//e_proto_cmd_findhip_reply = 11, // searching HIP - reply
		} ;
};

inline bool enum_is_valid_value(c_protocolv3::t_proto_cmd value) {
	switch (value) {
		case c_protocolv3::t_proto_cmd::e_proto_cmd_unused_old_version_0:
		case c_protocolv3::t_proto_cmd::e_proto_cmd_unused_old_version_1:
		case c_protocolv3::t_proto_cmd::e_proto_cmd_unused_old_version_2:
		case c_protocolv3::t_proto_cmd::e_proto_cmd_data_one_merit_clear:
		case c_protocolv3::t_proto_cmd::e_proto_cmd_data_cart:
		return true;
	}
	return false;
}

// ------------------------------------------------------------------

class c_protocol {
	public:
		constexpr static unsigned char current_version = 1;

		constexpr static unsigned char version_size = 1;
		constexpr static unsigned char cmd_size = 1;
		constexpr static unsigned char ttl_size = 1; // size of TTL header

		constexpr static unsigned char ttl_max_value_ever = 200; // no value bigger then that can ever appear, it would be low level error to let that happen
		constexpr static unsigned char ttl_max_accepted = 5; // how high can be the TTL requested by others that we can [normally?] accept

/*
Proxy format - the data to be sent on wire to peer:

- 1 byte protocol version
- 1 byte public command/data:

Protocol versions:
version 0 - not used
version 1 - (experimental) protocol

*/

enum class t_proto_cmd : unsigned char {
	e_proto_cmd_test0 = 0,
	e_proto_cmd_test1 = 1,
	e_proto_cmd_tunneled_data = 2, // the tunneled data are following
	e_proto_cmd_public_hi = 3, // simple public peering
	e_proto_cmd_public_ping_request = 4, // simple public ping to the peer
	e_proto_cmd_public_ping_reply = 5, // simple public ping to the peer
	e_proto_cmd_findhip_query = 10, // searching HIP - query
	e_proto_cmd_findhip_reply = 11, // searching HIP - reply
};
static bool command_is_valid_from_unknown_peer( t_proto_cmd cmd ); ///< is this command one that can come from an unknown peer (without any HIP and CA)


};


inline bool enum_is_valid_value(c_protocol::t_proto_cmd value) {
	switch (value) {
		case c_protocol::t_proto_cmd::e_proto_cmd_test0:
		case c_protocol::t_proto_cmd::e_proto_cmd_test1:
		case c_protocol::t_proto_cmd::e_proto_cmd_tunneled_data:
		case c_protocol::t_proto_cmd::e_proto_cmd_public_hi:
		case c_protocol::t_proto_cmd::e_proto_cmd_public_ping_request:
		case c_protocol::t_proto_cmd::e_proto_cmd_public_ping_reply:
		case c_protocol::t_proto_cmd::e_proto_cmd_findhip_query:
		case c_protocol::t_proto_cmd::e_proto_cmd_findhip_reply:
		return true;
	}
	return false;
}


// ------------------------------------------------------------------

#endif

