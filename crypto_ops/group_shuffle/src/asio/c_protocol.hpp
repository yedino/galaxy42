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

// TODO singleton?
class c_protocol {
	public:
		c_protocol();
		void send_packet(const t_nym_id &destination, const e_packet_type &packet_type, const std::string &data = "");
		void tick();
	private:
		struct s_packet {
			std::string m_data; // raw data
			e_packet_type m_packet_type;
			t_nym_id m_address = "null_address"; // input address for input packet, destination address for output
		};
		std::unique_ptr<c_api_tr> m_network_device;
		std::vector<t_nym_id> m_addressbook;
		std::vector<s_packet> m_inbox;
		std::vector<s_packet> m_outbox;
		
		void recv_packets(); // get all input packets from m_network_device
		s_packet deserialize_bin_data(const std::string &data);
		void process_input_packet(const s_packet &packet);
};

#endif // C_PROTOCOL_HPP
