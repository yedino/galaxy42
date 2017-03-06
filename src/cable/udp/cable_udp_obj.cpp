#include "cable_udp_obj.hpp"
#include "cable_udp_addr.hpp"

using namespace boost::asio::ip;

c_cable_udp::c_cable_udp()
:
	m_io_service(),
	m_socket(m_io_service)
{
}

void c_cable_udp::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	udp::endpoint destination_endpoint = (boost::any_cast<c_cable_udp_addr::t_addr>( dest.get_addrdata() ));
	m_socket.send_to(boost::asio::buffer(data, size), destination_endpoint);
}

void c_cable_udp::async_send_to(const c_cable_base_addr &dest, const unsigned char *data, size_t size, write_handler handler) {
	udp::endpoint destination_endpoint = (boost::any_cast<c_cable_udp_addr::t_addr>( dest.get_addrdata() ));
	m_socket.async_send_to(boost::asio::buffer(data, size), destination_endpoint,
		[handler = std::move(handler), data](const boost::system::error_code& error, std::size_t bytes_transferred) {
			handler(data, bytes_transferred);
		} // lambda
	);
}

size_t c_cable_udp::receive_from(c_cable_base_addr &source, unsigned char *const data, size_t size) {
	udp::endpoint source_endpoint;
	size_t readed_bytes = m_socket.receive_from(boost::asio::buffer(data, size), source_endpoint);
	source.init_addrdata(source_endpoint);
	return readed_bytes;
}

void c_cable_udp::async_receive_from(unsigned char *const data, size_t size, read_handler handler)
{
	udp::endpoint *source_addr_ptr = new udp::endpoint; // boost will write this after async
	// raw pointer because asio handler must meet the requirements of CopyConstructible types
	// will by deleted in handler(via unique_ptr)

	m_socket.async_receive_from(boost::asio::buffer(data, size), *source_addr_ptr,
		[handler_ = handler, data, source_addr_ptr]
		(const boost::system::error_code& error, std::size_t bytes_transferred)
		{
			std::unique_ptr<c_cable_base_addr> source_addr_cable = std::make_unique<c_cable_udp_addr>( *source_addr_ptr );
			delete source_addr_ptr;
			handler_(data, bytes_transferred, std::move(source_addr_cable));
		} // lambda
	);
}

void c_cable_udp::listen_on(c_cable_base_addr &local_address) {
	udp::endpoint local_endpoint = boost::any_cast<c_cable_udp_addr::t_addr>(local_address.get_addrdata());
	m_socket.bind(local_endpoint);
}
