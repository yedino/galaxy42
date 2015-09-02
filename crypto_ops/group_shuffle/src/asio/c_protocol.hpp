#ifndef C_PROTOCOL_HPP
#define C_PROTOCOL_HPP

#include <c_api_tr.hpp>
#include <c_api_wot.hpp>
#include <libs1.hpp>
#include "c_net_node.hpp"

#include <memory>
#include <string>
#include <vector>
#include <sstream>

#define _info(X) std::cout << __FILE__ << ":" << __LINE__ << " " << X << std::endl
#define _note(X) std::cout << __FILE__ << ":" << __LINE__ << " " << X << std::endl

enum e_packet_type {
	PING = 0,
	PONG = 1,
	HELLO = 2
};


// raw data = xx yyyy zzzzzz...
// xx = packet type
// yyyy = length of data
// zzzz.. = data

class c_protocol {
	public:
		c_protocol();
		void send_packet(const t_nym_id &destination, const e_packet_type &packet_type);
		void recv_packet();
	private:
		std::unique_ptr<c_api_tr> m_network_device;
		std::vector<t_nym_id> m_addressbook;
		
		struct s_packet {
			std::string m_data;
			e_packet_type m_packet_type;
		};
		
		s_packet deserialize_bin_data(const std::string &data);
		void process_packet(const s_packet &packet);
};

#endif // C_PROTOCOL_HPP
