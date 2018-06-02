#include "c_tuntap_netbsd_obj.hpp"

/////////////////////////////////////////////////////////////////////

c_tuntap_netbsd_obj::c_tuntap_netbsd_obj() :
	m_tun_fd(open("/dev/" IFNAME, O_RDWR)),
	m_io_service(),
	m_tun_stream(m_io_service, m_tun_fd) 
{
    _dbg1(__func__ << " Tuntap " << IFNAME << " opened at " << m_tun_fd << " descriptor");
    /* ???
    int i;
    
    _goal("Set flags: IFF_POINTOPOINT|IFF_MULTICAST");
    i = IFF_POINTOPOINT|IFF_MULTICAST;
    // multicast on
    if(ioctl(m_tun_fd, TUNSIFMODE, &i) == -1) {
        perror("TUNSIFMODE");
    }
    
    _goal("Set flags: TUNSLMODE");
    i = 0;
    // link layer mode off
    if(ioctl(m_tun_fd, TUNSLMODE, &i) == -1) {
        perror("TUNSLMODE");
    }
    
    _goal("Set flags: TUNSIFHEAD");
    i = 1;
    // multi-af mode on
    if(ioctl(m_tun_fd, TUNSIFHEAD, &i) == -1) {
        perror("TUNSIFHEAD");
    }
    */
    
    _fact("tuntap opened with m_tun_fd=" << m_tun_fd);
    _try_sys(m_tun_fd != -1);
    _check_sys(m_tun_stream.is_open());
    _goal("tuntap is opened correctly");
}

c_tuntap_netbsd_obj::c_tuntap_netbsd_obj(boost::asio::io_service &io_service) :
	m_tun_fd(open("/dev/" IFNAME, O_RDWR)),
	m_io_service(),
	m_tun_stream(io_service, m_tun_fd)
{
    // c_tuntap_linux_obj.cpp: 47-60
    
    _fact("tuntap opened with m_tun_fd=" << m_tun_fd);
    _try_sys(m_tun_fd != -1);
    _check_sys(m_tun_stream.is_open());
    _goal("tuntap is opened correctly");
}

c_tuntap_netbsd_obj::~c_tuntap_netbsd_obj() 
{
    _goal("Close " IFNAME);
    close(m_tun_fd);
}


size_t c_tuntap_netbsd_obj::send_to_tun(
        const unsigned char *data, 
        size_t size
) {
    try {
        return m_tun_stream.write_some(boost::asio::buffer(data, size));
    } catch (const std::exception &e) {
        _warn(e.what());
        return 0; // error
    }
}

size_t c_tuntap_netbsd_obj::send_to_tun_separated_addresses(
        const unsigned char *const data, 
        size_t size,
        const std::array<unsigned char, IPV6_LEN> &src_binary_address,
	const std::array<unsigned char, IPV6_LEN> &dst_binary_address
) {
        _check_input(size >= 8);
	std::array<boost::asio::const_buffer, 4> buffers;
	buffers.at(0) = boost::asio::buffer(data, 8); // version, traffic, flow label, payload length, next header, hop limit
	buffers.at(1) = boost::asio::buffer(src_binary_address.data(), src_binary_address.size());
	buffers.at(2) = boost::asio::buffer(dst_binary_address.data(), dst_binary_address.size());
	buffers.at(3) = boost::asio::buffer(data + 8, size - 8); // 8 bytes are filled in buffers.at(0)
	boost::system::error_code ec;
	return m_tun_stream.write_some(buffers, ec);
}

size_t c_tuntap_netbsd_obj::read_from_tun(
        unsigned char *const data, 
        size_t size
) {
    try {
        return m_tun_stream.read_some(boost::asio::buffer(data, size));
    } catch (const std::exception &e) {
        _warn(e.what());
        return 0; // error
    }
}

size_t c_tuntap_netbsd_obj::read_from_tun_separated_addresses(
        unsigned char *const data, 
        size_t size,
	std::array<unsigned char, IPV6_LEN> &src_binary_address,
	std::array<unsigned char, IPV6_LEN> &dst_binary_address
) {
    _check_input(size >= 8);
    // field sizes based on rfc2460
    // https://tools.ietf.org/html/rfc2460
    std::array<boost::asio::mutable_buffer, 4> buffers;
    buffers.at(0) = boost::asio::buffer(data, 8); // version, traffic, flow label, payload length, next header, hop limit
    buffers.at(1) = boost::asio::buffer(src_binary_address.data(), src_binary_address.size());
    buffers.at(2) = boost::asio::buffer(dst_binary_address.data(), dst_binary_address.size());
    buffers.at(3) = boost::asio::buffer(data + 8, size - 8); // 8 bytes are filled in buffers.at(0)
    try {
        return m_tun_stream.read_some(buffers) - src_binary_address.size() - dst_binary_address.size();
    } catch (const std::exception &e) {
        _warn(e.what());
    return 0;
    }
}

void c_tuntap_netbsd_obj::async_receive_from_tun(
        unsigned char *const data,
        size_t size,
        const c_tuntap_base_obj::read_handler & handler
) {
    auto asio_handler = [data, handler](const boost::system::error_code& error, std::size_t bytes_transferred) {
        handler(data, bytes_transferred, error);
    };
    return m_tun_stream.async_read_some(boost::asio::buffer(data, size), asio_handler);
}

void c_tuntap_netbsd_obj::set_tun_parameters(const std::array<unsigned char, IPV6_LEN> &binary_address,
                                            int prefix_len,
                                            uint32_t mtu)
{
    /*
    int sock, inetret;
    uint16_t scope;
    
    _goal("Create socket ...");
    sock = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW); // create socket, opts ???
    
    struct in6_aliasreq ifa6;
    memset(&ifa6, 0, sizeof(struct in6_aliasreq));
    
    strncpy(ifa6.ifra_name, IFNAME, sizeof(ifa6.ifra_name)); // if name
    
    ifa6.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;
    ifa6.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
    
    // address
    ifa6.ifra_addr.sin6_family = AF_INET6;
    ifa6.ifra_addr.sin6_len = sizeof(struct sockaddr_in6);
    inetret = inet_pton(AF_INET6, "fe80::01", &ifa6.ifra_addr.sin6_addr);
    if(inetret == -1) {
        perror("inet_pton: addr");
    }
    
    // destination address
    //ifa6.ifra_dstaddr.sin6_family = AF_INET6;
    //ifa6.ifra_dstaddr.sin6_len = sizeof(struct sockaddr_in6);
    //inetret = inet_pton(AF_INET6, "fe80::02", &ifa6.ifra_dstaddr.sin6_addr);
    //if(inetret == -1) {
    //    perror("inet_pton: dstaddr");
    //}
    
    // netmask - prefixlen
    ifa6.ifra_prefixmask.sin6_family = AF_INET6;
    ifa6.ifra_prefixmask.sin6_len = sizeof(struct sockaddr_in6);
    ipv6_mask(&ifa6.ifra_prefixmask.sin6_addr, prefix_len);
    
    // scope id in address
    scope = htons((uint16_t)if_nametoindex(IFNAME));
    struct in6_addr *s6a1 = &ifa6.ifra_addr.sin6_addr;
    memcpy(&s6a1[2], &scope, sizeof(uint16_t));
    ifa6.ifra_addr.sin6_scope_id = 0;
    
    _goal("Setting addresses ...");
    // call add address
    if(ioctl(sock, SIOCAIFADDR_IN6, &ifa6) == -1) {
        perror("SIOCAIFADDR_IN6");
    }
    
    struct ifreq interface;
    
    memset(&interface, 0, sizeof(struct ifreq));
    
    _goal("Create socket ...");
    
    strncpy(interface.ifr_name, IFNAME, sizeof(interface.ifr_name)); // if name
    interface.ifr_ifru.ifru_mtu = mtu;
    
    _goal("Setting MTU on " << IFNAME << " to " << mtu);
    if(ioctl(sock, SIOCSIFMTU, &interface) == -1) {
        _erron("SIOCSIFMTU");
    }
    
    close(sock);
    */
    
    tundn.set_ipv6_address(binary_address, prefix_len);
    tundn.set_mtu(mtu);
    
    m_tun_stream.release();
    m_tun_stream.assign(m_tun_fd);
    _goal("Configuring tuntap options - done");
        
}
