#include "c_netdev.hpp"
#include "libs1.hpp"
#if defined USE_API_TR
/*
void c_netdev::receive_message (unique_ptr<c_msgtx> &&message) {
	m_inbox.emplace_back(std::move(message));
}

unique_ptr<c_msgtx> c_netdev::send_message () {
	if (m_outbox.empty()) {
		return nullptr;
	}
	unique_ptr<c_msgtx> ret_value = std::move(m_outbox.at(0));
	m_outbox.erase(m_outbox.begin());
	return ret_value;
}

// ==================================================================

c_netdev::c_netdev (string name, t_pos x, t_pos y) : c_entity(name, x, y) {


}
*/


void c_netdev::hw_send (t_message msg) {
	m_outbox.push_back(msg);
}

vector<t_message> c_netdev::hw_recived()
{
	auto ret_vect(m_inbox);
	m_inbox.clear();
	return ret_vect;

}

void c_netdev::set_network(std::shared_ptr<c_network> network_ptr) {
    m_network = network_ptr;
}

/*
void c_netdev::tick()
{

}
*/

#endif