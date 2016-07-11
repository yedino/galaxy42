#ifndef C_UDP_WRAPPER_HPP
#define C_UDP_WRAPPER_HPP


#include "c_ip46_addr.hpp" // TODO make portable
#include "c_event_manager.hpp"

class c_udp_wrapper {
	public:
		virtual ~c_udp_wrapper() = default;
		virtual void send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) = 0;
		virtual size_t
			receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) = 0;
};

#ifdef __linux__
class c_udp_wrapper_linux final : public c_udp_wrapper {
	friend class c_event_manager_linux;
	public:
		c_udp_wrapper_linux(const int listen_port);
		void send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) override;
		size_t receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) override;
		int get_socket(); // TODO remove this
	private:
		const int m_socket;
};

#else

class c_udp_wrapper_empty final : public c_udp_wrapper {
	public:
		c_udp_wrapper_empty(const int listen_port);
		void send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) override;
		size_t receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) override;
		int get_socket() { return -1; } // TODO remove this
};

#endif // __linux__

#endif // C_UDP_WRAPPER_HPP
