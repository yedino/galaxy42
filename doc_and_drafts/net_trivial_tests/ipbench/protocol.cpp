
#include "protocol.hpp"

bool c_protocol::command_is_valid_from_unknown_peer( c_protocol::t_proto_cmd cmd ) {
	if (cmd == e_proto_cmd_tunneled_data) return false; // most common case

	if (cmd == e_proto_cmd_public_hi) return true; // establishes CA
	if (cmd == e_proto_cmd_public_ping_request) return true; // ok to unauthed
	if (cmd == e_proto_cmd_public_ping_reply) return true; // ok to unauthed

	return false;
}

