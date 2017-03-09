#ifndef C_TURBOSOCKET_HPP
#define C_TURBOSOCKET_HPP

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <tuple>

class c_turbosocket final {
	private:
		struct header;
	public:
		c_turbosocket() = default;
		c_turbosocket(c_turbosocket && other) = default;
		c_turbosocket(const c_turbosocket &) = delete;

		std::tuple<void *, size_t> get_buffer_for_write_to_server(); ///< block until buffer ready
		std::tuple<void *, size_t> get_buffer_for_write_to_client(); ///< block until buffer ready
		std::tuple<void *, size_t> get_buffer_for_read_from_server(); ///< block until buffer ready
		std::tuple<void *, size_t> get_buffer_for_read_from_client(); ///< block until buffer ready
		void send_to_server(size_t size, const unsigned char dst_address[16], unsigned short dst_port);
		void send_to_client(size_t size, const unsigned char dst_address[16], unsigned short dst_port);
		void received_from_server();
		void received_from_client();

		void connect_as_client();
		void wait_for_connection(); // block function
		bool timed_wait_for_connection(); ///< wait with timeout, return true if connected
		/**
		 * @brief ready_for_read
		 * check if data is ready for read, if return true get_buffer_for_read() returns immediately
		 * not blocks
		 */
		bool server_data_ready_for_read();
		uint64_t id() const;
		const std::array<unsigned char, 16> &get_srv_ipv6() const; ///< returns destination address from server header
		const std::array<unsigned char, 16> &get_cli_ipv6() const; ///< returns source address from client header
		unsigned short get_srv_port() const;
		unsigned short get_cli_port() const;
	private:
		struct header {
			boost::interprocess::interprocess_mutex mutex;
			// wait when the queue is full
			boost::interprocess::interprocess_condition cond_full;
			// wait when the queue is empty
			boost::interprocess::interprocess_condition cond_empty;
			//Is there any message
			bool message_in = false;
			// socket unique ID
			uint64_t id = 0;
			// size of data
			size_t data_size;

			std::array<unsigned char, 16> destination_ipv6;
			std::array<unsigned char, 16> source_ipv6;
			unsigned short destination_port;
			unsigned short source_port;
		};

		uint64_t get_uid() const;

		const std::string m_queue_name = "tunserver_turbosocket_queue";
		static constexpr size_t m_max_queue_massage_size = 20;
		const size_t m_size_of_single_buffer = 65 * 1024;
		const size_t m_shm_size = (m_size_of_single_buffer + sizeof(header)) * 2; // max MTU + header for 2 direction

		boost::interprocess::mapped_region m_shm_region_client_to_server; ///< for sending messages from client to server
		boost::interprocess::mapped_region m_shm_region_server_to_client; ///< for sending messages from server to client
		header * m_header_client_to_server;
		header * m_header_server_to_client;
		void create_shm(const char *name);
		void open_shm(const char *name);

		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> m_lock_client_to_server;
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> m_lock_server_to_client;
		uint64_t m_id = 0;
};

#endif // C_TURBOSOCKET_HPP
