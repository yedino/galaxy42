#ifndef C_NETWORK_HPP
#define C_NETWORK_HPP

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include "c_api_tr.hpp"
#include "c_object.hpp"

class c_cjddev;
class c_api_tr;
struct t_message;
struct t_hw_message;

// "network lib"
class c_network : public std::enable_shared_from_this<c_network> {
	private:
		std::vector<std::pair<t_nym_id, std::string> > m_message_vector;
		std::map<t_nym_id, std::shared_ptr<c_cjddev>> m_node_map;
	public:
		void add_node(std::shared_ptr<c_cjddev> node); ///< add new node to m_node_map
		void send_message(t_nym_id addr, std::string &&message); ///< add message to m_message_vector
		void tick(); ///< process all messages
		
};

#endif // C_NETWORK_HPP
