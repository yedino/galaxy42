#include "c_network.hpp"

void c_network::add_node (std::shared_ptr<c_cjddev> node) {
	//node->set_network(shared_from_this()); TODO
	t_nym_id node_address = node->get_address();
    //m_node_map.emplace(node_address, std::move(node));
    m_node_map[node_address] = std::move(node);

}
