#include "c_network.hpp"

void c_network::add_node (std::shared_ptr<c_cjddev> node) {
	//node->set_network(shared_from_this()); TODO
	t_nym_id node_address = node->get_address();
    //m_node_map.emplace(node_address, std::move(node));
    m_node_map[node_address] = std::move(node);

}
#if defined USE_API_TR
void c_network::send_message (t_nym_id addr,std::string &&message) {
	_note("c_network::send_message()");
	_note("message size: " << message.size());
	m_message_vector.emplace_back(std::make_pair( addr,message) );
}

void c_network::tick() {
//	_note("c_network::tick()");
	for(auto node : m_node_map){
		auto &messages = node.second->m_netdev->m_outbox;
		for (auto msg :messages){
			m_node_map.at(msg.m_remote_id)->m_netdev->m_inbox.push_back(msg);
		}
		messages.clear();
	}
/*
	for(auto node : m_node_map){
		node.second->tick();
	}
*/
	/*
	for (auto &msg : m_message_vector) {
		if(!m_node_map.empty()){
			//m_node_map.at(msg.first)->hw_recived(std::move(msg.second));

		}
	*/
//	m_message_vector.clear();
//m_node_map.at(msg.m_msg.m_remote_id)->write_message(std::move(msg.m_msg));
//	}

}
#endif
