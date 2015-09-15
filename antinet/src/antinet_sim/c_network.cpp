#include "c_network.hpp"

void c_network::add_node (std::shared_ptr<c_cjddev> node) {
	t_nym_id node_address = std::to_string(node->get_address());
	m_node_map.emplace(node_address, std::move(node));
}

void c_network::send_message (t_message &&message) {
	t_hw_message hw_message;
	hw_message.m_type = 0; // TODO
	hw_message.m_msg = message;
	m_message_vector.emplace_back(hw_message);
}
