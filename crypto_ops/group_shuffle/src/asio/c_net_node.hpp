#ifndef C_NET_NODE_HPP
#define C_NET_NODE_HPP

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <c_api_tr.hpp>
#include "c_session.hpp"

#define _info(X) std::cout << __FILE__ << ":" << __LINE__ << " " << X << std::endl
#define _note(X) std::cout << __FILE__ << ":" << __LINE__ << " " << X << std::endl

#define MAX_RECV_DATA_SIZE 1024

class c_net_node : public c_api_tr {
	public:
		c_net_node();
		~c_net_node();
		void run();
		virtual void write_to_nym(t_nym_id guy, const std::string & data) override; // guy = ipv6, thread unsafe
		virtual vector<s_message> read_or_wait_for_data() override; // thread safe, clear buffer
		void add_to_inbox(char *data, size_t size, const boost::asio::ip::address &source_address); // thread safe
	private:
		const unsigned short m_port = 4000; // port for server
		boost::asio::io_service m_io_service;
		std::vector < std::unique_ptr < std::thread > > m_thread_vector; // threads for io_service
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		boost::asio::ip::tcp::socket m_client_socket; // TODO mutex?
		boost::asio::deadline_timer m_timer;
		const unsigned int m_number_of_threads = 5;
		char m_recv_data[MAX_RECV_DATA_SIZE];
		std::vector<s_message> m_inbox;
		std::mutex m_inbox_mutex;
		void do_accept();
};

#endif // C_NET_NODE_HPP
