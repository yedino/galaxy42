#ifndef C_UDP_WRAPPER_HPP
#define C_UDP_WRAPPER_HPP


#include "c_ip46_addr.hpp" // TODO make portable

class c_udp_wrapper {
	public:
		virtual ~c_udp_wrapper() = default;
		virtual void send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) = 0;
		virtual void receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) = 0;
};

#ifdef __linux__
class c_udp_wrapper_linux final : public c_udp_wrapper {
	public:
		c_udp_wrapper_linux(const int listen_port);
		void send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) override;
	private:
		int m_socket;
};
#endif // __linux__

#endif // C_UDP_WRAPPER_HPP
