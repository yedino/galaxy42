// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

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

typedef enum {
	e_proto_cmd_test0 = 0,
	e_proto_cmd_test1 = 1,
	e_proto_cmd_tunneled_data = 2, // the tunneled data are following
	e_proto_cmd_public_hi = 3, // simple public peering
	e_proto_cmd_public_ping_request = 4, // simple public ping to the peer
	e_proto_cmd_public_ping_reply = 5, // simple public ping to the peer
	e_proto_cmd_findhip_query = 10, // searching HIP - query
	e_proto_cmd_findhip_reply = 11, // searching HIP - reply
} t_proto_cmd ;

static bool command_is_valid_from_unknown_peer( t_proto_cmd cmd ); ///< is this command one that can come from an unknown peer (without any HIP and CA)


};

// ------------------------------------------------------------------

#endif

