#include "c_protocol.hpp"

c_protocol::c_protocol() :
	m_network_device(make_unique<c_net_node>())
{
}


void c_protocol::send_packet(const t_nym_id &destination_address, const e_packet_type &packet_type) {
	_info("generate packet data");
	std::string data;
	if (packet_type == PING) {
	}
	else if (packet_type == PONG) {
	}
	else if (packet_type == HELLO) {
	}
	_info("start serialize");
	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	uint16_t uint_type = packet_type;
	_info("packet_type " << packet_type);
	oss.write(reinterpret_cast<char *>(&uint_type), sizeof(uint_type));
	
	uint32_t uint_data_size = data.size();
	oss.write(reinterpret_cast<char *>(&uint_data_size), sizeof(uint_data_size));
	oss << data;
	
	_info("end of serialization");
	_info("send data");
	m_network_device->write_to_nym(destination_address, oss.str());
}

void c_protocol::recv_packet() {
	std::vector<s_message> recv_messages = m_network_device->read_or_wait_for_data();
	for (auto &msg : recv_messages) {
		process_packet(deserialize_bin_data(msg.m_data));
	}
}

c_protocol::s_packet c_protocol::deserialize_bin_data(const std::string &data) {
	std::istringstream iss(data, std::ios_base::in | std::ios_base::binary);
	uint16_t uint_type;
	uint32_t uint_data_size;
	std::string clear_data;
	
	iss.read(reinterpret_cast<char *>(&uint_type), sizeof(uint_type));
	iss.read(reinterpret_cast<char *>(&uint_data_size), sizeof(uint_data_size));
	iss >> clear_data;
	
	s_packet packet;
	packet.m_data = std::move(clear_data);
	packet.m_packet_type = static_cast<e_packet_type>(uint_type);
	return packet;
}

void c_protocol::process_packet(const c_protocol::s_packet &packet) {
	// TODO !!!
}
