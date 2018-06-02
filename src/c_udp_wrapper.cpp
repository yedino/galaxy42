#include "c_udp_wrapper.hpp"
#include "tnetdbg.hpp"
#include "c_ip46_addr.hpp"

c_udp_wrapper::c_udp_wrapper()
	: m_disabled(false)
{ }


#ifdef __linux__

c_udp_wrapper_linux::c_udp_wrapper_linux(const int listen_port)
:
	m_socket(socket(AF_INET, SOCK_DGRAM, 0))
{
	_note("Creating udp wrapper (old-style)");
	if (listen_port==0) {
		m_disabled=true;
		_warn("Socket is disabled, listen_port="<<listen_port);
		return;
	}
	_assert(m_socket >= 0);
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(listen_port);
	int bind_result = -1;
	if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
		sockaddr_in addr4 = address_for_sock.get_ip4();
		_note("Binding");
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
	}
	else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
		sockaddr_in6 addr6 = address_for_sock.get_ip6();
		_note("Binding");
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
	}
	_assert( bind_result >= 0 ); // TODO change to except
	_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
}

void c_udp_wrapper_linux::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	if (m_disabled) { _dbg4("disabled socket"); return; }
	auto dst_ip4 = dst_address.get_ip4(); // ip of proper type, as local variable
	sendto(m_socket, data, size_of_data, 0, reinterpret_cast<sockaddr*>(&dst_ip4), sizeof(sockaddr_in));
}

size_t c_udp_wrapper_linux::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	if (m_disabled) { _dbg4("disabled socket"); return 0; }
	sockaddr_in6 from_addr_raw; // peering address of peer (socket sender), raw format
	socklen_t from_addr_raw_size = sizeof(from_addr_raw); // ^ size of it
	auto size_read = recvfrom(m_socket, data_buf, data_buf_size, 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
	if (from_addr_raw_size == sizeof(sockaddr_in6)) { // the message arrive from IP pasted into sockaddr_in6 format
		_erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
		// trivial
	}
	else if (from_addr_raw_size == sizeof(sockaddr_in)) { // the message arrive from IP pasted into sockaddr_in (ipv4) format
		sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
		from_address.set_ip4(addr);
	} else {
		_throw_error( std::runtime_error("Data arrived from unknown socket address type") );
	}
	return size_read;
}

int c_udp_wrapper_linux::get_socket() {
	if (m_disabled) { _dbg4("disabled socket"); return 0 ; }
	return m_socket;
}


// __linux__
#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MACH__) // (multiplatform boost::asio)

#include <boost/bind.hpp>
#if defined (__MINGW32__)
	#undef _assert
#endif

c_udp_wrapper_asio::c_udp_wrapper_asio(const int listen_port)
:
	m_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)),
	m_bytes_readed(0)
{
	_fact("udp wrapper constructor, listen port " << listen_port);
	if (listen_port==0) {
		m_disabled=true;
		_warn("Socket is disabled, listen_port="<<listen_port);
		return;
	}
	//m_socket.async_receive_from(boost::asio::buffer(m_buffer),
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_asio::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void c_udp_wrapper_asio::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	if (m_disabled) { _dbg4("disabled socket"); return ; }
//	std::cout << "send udp packet, size " << size_of_data << std::endl;
	_dbg1("udp sending data to [" << dst_address << "]" << " size [" << size_of_data << "]");
	m_socket.send_to( // blocks
			boost::asio::buffer(data, size_of_data),
			boost::asio::ip::udp::endpoint(dst_address.get_address(), dst_address.get_assigned_port())
	);
}

size_t c_udp_wrapper_asio::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	if (m_disabled) { _dbg4("disabled socket"); return 0 ; }
	//std::cout << "udp receive data" << std::endl;
    if (m_bytes_readed > 0) { // readed data in m_buffer
		if (m_bytes_readed > data_buf_size) throw std::runtime_error("undersized buffer");
//		std::copy_n(m_buffer.begin(), m_bytes_readed, data_buf);
		std::copy_n(&m_buffer[0], m_bytes_readed, reinterpret_cast<uint8_t *>(data_buf));
		size_t ret = m_bytes_readed;
		from_address.set_address(m_sender_endpoint.address());
		from_address.set_port(m_sender_endpoint.port());
		_dbg1("udp recieving data from [" << from_address << "]");
		m_bytes_readed = 0;
		return ret;
	}
	return 0;
}

void c_udp_wrapper_asio::read_handle(const boost::system::error_code& error, size_t bytes_transferred) {
	if (m_disabled) { _dbg4("disabled socket"); return ; }
//	std::cout << "udp read handle" << std::endl;
//	std::cout << "readed " << bytes_transferred << " bytes" << std::endl;
	if (error && (error.value() != 10061)) { // http://stackoverflow.com/a/16763404
		_warn("UDP read error: " << error.message() << " value " << error.value());
	}
//	assert(m_bytes_readed == 0);
	m_bytes_readed = bytes_transferred;
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_asio::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

// __win32 || __cygwin__ || __mach__ (multiplatform boost::asio)
#elif defined(__NetBSD__)

c_udp_wrapper_netbsd::c_udp_wrapper_netbsd(const int listen_port)
:
	m_socket(socket(AF_INET, SOCK_RAW, 0))
{
        //int state = 1;
        //setsockopt(m_socket, 0, IP_HDRINCL, &state, sizeof(state));
	_note("Creating udp wrapper (old-style)");
	if (listen_port==0) {
		m_disabled=true;
		_warn("Socket is disabled, listen_port="<<listen_port);
		return;
	}
	_assert(m_socket >= 0);
        c_ip46_addr anyport;
	c_ip46_addr address_for_sock = anyport.any_on_port(listen_port);
	int bind_result = -1;
	if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
		sockaddr_in addr4 = address_for_sock.get_ip4();
		_note("Binding 4");
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
	}
	else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
		sockaddr_in6 addr6 = address_for_sock.get_ip6();
		_note("Binding 6");
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
	}
        struct ifreq ifr;
        memset (&ifr, 0, sizeof (ifr));
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", IFNAME);
        if (ioctl (m_socket, SIOCGIFINDEX, &ifr) < 0) {
            std::stringstream errorstring;
            char *serr = strerror(errno);
            errorstring<<"SIOCGIFINDEX : "<<serr;
            _erro(errorstring.str());
            _throw_error_sub( tuntap_error_devtun , errorstring.str() );
        } else {
            _dbg5("SIOCGIFINDEX : " << ifr.ifr_ifindex);
        }
        if(bind_result == -1) {
            char *serr = strerror(errno);
            std::stringstream errorstring;
            errorstring<<"ERRNO = "<<serr<<" , Some possible solutions: ";
            switch(errno) {
                case EINVAL:
                    errorstring<<"You have invalid argument in bind()";
                    break;
                default:
                    errorstring<<"Please describe errno "<<errno;
            }
            _warnn(errorstring.str());
            _throw_error_sub( tuntap_error_devtun , errorstring.str() );
        }
	_assert( bind_result >= 0 ); // TODO change to except
	_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
}

void c_udp_wrapper_netbsd::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	if (m_disabled) { _dbg4("disabled socket"); return; }
	auto dst_ip4 = dst_address.get_ip4(); // ip of proper type, as local variable
	sendto(m_socket, data, size_of_data, 0, reinterpret_cast<sockaddr*>(&dst_ip4), sizeof(sockaddr_in));
}

size_t c_udp_wrapper_netbsd::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	if (m_disabled) { _dbg4("disabled socket"); return 0; }
	sockaddr_in6 from_addr_raw; // peering address of peer (socket sender), raw format
	socklen_t from_addr_raw_size = sizeof(from_addr_raw); // ^ size of it
	auto size_read = recvfrom(m_socket, data_buf, data_buf_size, 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
	if (from_addr_raw_size == sizeof(sockaddr_in6)) { // the message arrive from IP pasted into sockaddr_in6 format
		_erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
		// trivial
	}
	else if (from_addr_raw_size == sizeof(sockaddr_in)) { // the message arrive from IP pasted into sockaddr_in (ipv4) format
		sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
		from_address.set_ip4(addr);
	} else {
		_throw_error( std::runtime_error("Data arrived from unknown socket address type") );
	}
	return size_read;
}

int c_udp_wrapper_netbsd::get_socket() {
	if (m_disabled) { _dbg4("disabled socket"); return 0 ; }
	return m_socket;
}

#if 0
#include <boost/bind.hpp>

c_udp_wrapper_asio::c_udp_wrapper_asio(const int listen_port)
:
	m_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)),
	m_bytes_readed(0)
{
	_fact("udp wrapper constructor, listen port " << listen_port);
	if (listen_port==0) {
		m_disabled=true;
		_warn("Socket is disabled, listen_port="<<listen_port);
		return;
	}
	//m_socket.async_receive_from(boost::asio::buffer(m_buffer),
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_asio::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void c_udp_wrapper_asio::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) { // XXX
	if (m_disabled) { _dbg4("disabled socket"); return ; }
	_dbg1("udp sending data to " << dst_address << " [size:" << size_of_data << "]");
        m_socket.send_to( // blocks
			boost::asio::buffer(data, size_of_data),
			boost::asio::ip::udp::endpoint(dst_address.get_address(), dst_address.get_assigned_port())
	);
}

size_t c_udp_wrapper_asio::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) { // XXX
        if (m_disabled) { _dbg4("disabled socket"); return 0; }
	//auto size_read = recvfrom(, data_buf, data_buf_size, 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
	auto buffer = boost::asio::buffer(data_buf, data_buf_size);
        boost::asio::ip::udp::endpoint sender_endpoint;
        std::size_t size_read = m_socket.receive_from(buffer, sender_endpoint);
        const boost::asio::ip::address adr = sender_endpoint.address();
        boost::asio::ip::address_v4 a4;
        boost::asio::ip::address_v6 a6;
        if(adr.is_v4()) {
            a4 = adr.to_v4();
            uint32_t uia = a4.to_uint();
            sockaddr_in from_addr_raw;
            memcpy(&from_addr_raw.sin_addr, &uia, sizeof(struct in_addr));
            sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
            from_address.set_ip4(addr);
        } else if(adr.is_v6()) {
            a6 = adr.to_v6();
            _erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
            // trivial
        } else {
            _throw_error( std::runtime_error("Data arrived from unknown socket address type") );
        }
	return size_read;	
    
        /* if (m_disabled) { _dbg4("disabled socket"); return 0 ; }
	//std::cout << "udp receive data" << std::endl;
        if (m_bytes_readed > 0) { // readed data in m_buffer
		if (m_bytes_readed > data_buf_size) throw std::runtime_error("undersized buffer");
//		std::copy_n(m_buffer.begin(), m_bytes_readed, data_buf);
		std::copy_n(&m_buffer[0], m_bytes_readed, reinterpret_cast<uint8_t *>(data_buf));
		size_t ret = m_bytes_readed;
		from_address.set_address(m_sender_endpoint.address());
		from_address.set_port(m_sender_endpoint.port());
		_dbg1("udp recieving data from [" << from_address << "]");
		m_bytes_readed = 0;
		return ret;
	}
	return 0; */
}

void c_udp_wrapper_asio::read_handle(const boost::system::error_code& error, size_t bytes_transferred) {
	if (m_disabled) { _dbg4("disabled socket"); return ; }
//	std::cout << "udp read handle" << std::endl;
//	std::cout << "readed " << bytes_transferred << " bytes" << std::endl;
	if (error && (error.value() != 10061)) { // http://stackoverflow.com/a/16763404
		_warn("UDP read error: " << error.message() << " value " << error.value());
	}
//	assert(m_bytes_readed == 0);
	m_bytes_readed = bytes_transferred;
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_asio::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
#endif

#else

c_udp_wrapper_empty::c_udp_wrapper_empty(const int listen_port) {
	_UNUSED(listen_port);
	_dbg4("unimplemented socket");
}

void c_udp_wrapper_empty::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	_UNUSED(dst_address);
	_UNUSED(data);
	_UNUSED(size_of_data);
	_dbg4("unimplemented socket");
}

size_t c_udp_wrapper_empty::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	_UNUSED(data_buf);
	_UNUSED(data_buf_size);
	_UNUSED(from_address);
	_dbg4("unimplemented socket");
	return 0;
}

// else
#endif
