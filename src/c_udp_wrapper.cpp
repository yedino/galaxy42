#include "c_udp_wrapper.hpp"


#ifdef __linux__
c_udp_wrapper_linux::c_udp_wrapper_linux(const int listen_port) {
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	_assert(m_socket >= 0);
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(listen_port);
	int bind_result = -1;
	if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
		sockaddr_in addr4 = address_for_sock.get_ip4();
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
	}
	else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
		sockaddr_in6 addr6 = address_for_sock.get_ip6();
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
	}
		_assert( bind_result >= 0 ); // TODO change to except
		_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
}

void c_udp_wrapper_linux::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	auto dst_ip4 = dst_address.get_ip4(); // ip of proper type, as local variable
	sendto(m_socket, data, size_of_data, 0, reinterpret_cast<sockaddr*>(&dst_ip4), sizeof(sockaddr_in));
}
#endif // __linux__
