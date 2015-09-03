#include "c_protocol.hpp"

c_protocol::c_protocol() :
	m_network_device(make_unique<c_net_node>())
{
}

void c_protocol::send_packet(const t_nym_id &destination_address, const e_packet_type &packet_type, const std::string &data) {
	_info("start serialize");
	std::ostringstream oss(std::ios_base::out | std::ios_base::binary);
	uint16_t uint_type = static_cast<uint16_t>(packet_type);
	_info("packet_type " << packet_type);
	oss.write(reinterpret_cast<char *>(&uint_type), sizeof(uint_type));
	
	uint32_t uint_data_size = data.size();
	oss.write(reinterpret_cast<char *>(&uint_data_size), sizeof(uint_data_size));
	oss << data;
	_info("end of serialization");
	
	_info("add to outbox");
	s_packet raw_packet;
	raw_packet.m_data = std::move(oss.str());
	raw_packet.m_packet_type = packet_type;
	raw_packet.m_address = destination_address;
	m_outbox.emplace_back(std::move(raw_packet));
}

void c_protocol::recv_packets() {
	std::vector<s_message> recv_messages = m_network_device->read_or_wait_for_data();
	for (auto &msg : recv_messages) {
		s_packet packet = std::move(deserialize_bin_data(msg.m_data));
		packet.m_address = std::move(msg.m_source_id);
		m_inbox.emplace_back(std::move(packet));
	}
}

void c_protocol::tick() {
	recv_packets();
	for (auto &input_packet : m_inbox) {
		process_input_packet(input_packet);
	}
	for (auto &output_packet : m_outbox) {
		_info("send packet to " << output_packet.m_address);
		m_network_device->write_to_nym(output_packet.m_address, output_packet.m_data);
	}
	m_outbox.clear();
	m_inbox.clear();
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

void c_protocol::process_input_packet(const c_protocol::s_packet &packet) {
	_info("process_input_packet");
	_info(packet.m_address);
	_info(packet.m_packet_type);
	_info("**************************");
	if (packet.m_packet_type == e_packet_type::PING) {
		send_packet(packet.m_address, e_packet_type::PONG);
	}
	else if (packet.m_packet_type == e_packet_type::PONG) {
		std::cout << "***********************get pong msg from " << packet.m_address << std::endl;
	}
}
