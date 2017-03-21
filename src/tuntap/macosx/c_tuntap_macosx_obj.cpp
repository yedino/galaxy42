#include "c_tuntap_macosx_obj.hpp"
#ifdef ANTINET_macosx

#include "../../cpputils.hpp"
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include "../../haship.hpp"

c_tuntap_macosx_obj::c_tuntap_macosx_obj() : m_tun_fd(create_tun_fd()),
                                             m_io_service(),
                                             m_tun_stream(m_io_service, m_tun_fd) {

	_fact("Creating the MAC OS X tuntap_obj class (in ctor)");
	_fact("Tuntap opened with m_tun_fd=" << m_tun_fd);

	_try_sys(m_tun_fd != -1);
	_check_sys(m_tun_stream.is_open());
	_goal("Tuntap opened correctly");
}

size_t c_tuntap_macosx_obj::send_to_tun(const unsigned char *data, size_t size) {
/* OLD STYLE todo - waiting for memory model
size_t c_tun_device_apple::write_to_tun(void *buf, size_t count) {
    boost::system::error_code ec;
    uint8_t *buf_ptr = static_cast<uint8_t *>(buf);
    // TUN HEADER
    buf_ptr[0] = 0x00;
    buf_ptr[1] = 0x00;
    buf_ptr[2] = 0x00;
    buf_ptr[3] = 0x1E;

    //size_t write_bytes = m_stream_handle_ptr->write_some(boost::asio::buffer(buf, count), ec);
    size_t write_bytes = write(*m_stream_handle_ptr.get(), boost::asio::buffer(buf, count), ec);
    if (ec) throw std::runtime_error("write to TUN error: " + ec.message());
    return write_bytes;
}
*/
	return m_tun_stream.write_some(boost::asio::buffer(data, size));
}

size_t c_tuntap_macosx_obj::read_from_tun(unsigned char * const data, size_t size) {
/* OLD STYLE todo - waiting for memory model
size_t c_tun_device_apple::read_from_tun(void *buf, size_t count) {
    assert(m_readed_bytes > 0);
    if(m_readed_bytes > count) throw std::runtime_error("undersized buffer");
    // TUN header
    m_buffer[0] = 0x00;
    m_buffer[1] = 0x00;
    m_buffer[2] = 0x86;
    m_buffer[3] = 0xDD;
    std::copy_n(&m_buffer[0], m_readed_bytes, static_cast<uint8_t *>(buf));
    size_t ret = m_readed_bytes;
    m_readed_bytes = 0;
    return ret;
}
*/
	return m_tun_stream.read_some(boost::asio::buffer(data, size));
}

void c_tuntap_macosx_obj::async_receive_from_tun(unsigned char * const data,
                                                 size_t size,
                                                 const c_tuntap_base_obj::read_handler &handler) {

	auto asio_handler = [data, handler](const boost::system::error_code& error, std::size_t bytes_transferred) {
		handler(data, bytes_transferred, error);
	};
	return m_tun_stream.async_read_some(boost::asio::buffer(data, size), asio_handler);
}

void c_tuntap_macosx_obj::set_tun_parameters(const std::array<unsigned char, 16> &binary_address,
                                             int prefix_len,
                                             uint32_t mtu) {

	c_haship_addr address(c_haship_addr::tag_constr_by_array_uchar(), binary_address);
	_goal("Configuring tuntap options: IP address: " << address << "/" << prefix_len << " MTU=" << mtu);

	set_ipv6_address(binary_address, prefix_len);
	set_mtu(mtu);

	m_tun_stream.release();
	m_tun_stream.assign(m_tun_fd);
	_goal("Configuring tuntap options - done");
}

int c_tuntap_macosx_obj::create_tun_fd() {
	int tun_fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
	int err=errno;
	if (tun_fd < 0)
		_throw_error_sub(tuntap_error_devtun,
		                 NetPlatform_syserr_to_string({e_netplatform_err_open_fd, err}) );

	// get ctl_id
	ctl_info info;
	std::memset(&info, 0, sizeof(info));
	const std::string apple_utun_control = "com.apple.net.utun_control";
	apple_utun_control.copy(info.ctl_name, apple_utun_control.size());
	if (ioctl(tun_fd,CTLIOCGINFO, &info) < 0) { // errno
		int err = errno;
		close(tun_fd);
		_throw_error_sub(tuntap_error_devtun,
		                 NetPlatform_syserr_to_string({e_netplatform_err_open_fd, err}) );
	}

	// connect to tun
	sockaddr_ctl addr_ctl;
	addr_ctl.sc_id = info.ctl_id;
	addr_ctl.sc_len = sizeof(addr_ctl);
	addr_ctl.sc_family = AF_SYSTEM;
	addr_ctl.ss_sysaddr = AF_SYS_CONTROL;
	addr_ctl.sc_unit = 1;

	// connect to first not used tun
	int tested_card_counter = 0;
	auto t0 = std::chrono::system_clock::now();
	_fact(mo_file_reader::gettext("L_searching_for_virtual_card"));
	while (connect(tun_fd, reinterpret_cast<sockaddr *>(&addr_ctl), sizeof(addr_ctl)) < 0) {
		auto int_s = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - t0).count();
		if (tested_card_counter++ > number_of_tested_cards)
			_throw_error_sub(tuntap_error_devtun,
			                 mo_file_reader::gettext("L_max_number_of_tested_cards_limit_reached"));
		if (int_s >= cards_testing_time)
			_throw_error_sub( tuntap_error_devtun, mo_file_reader::gettext("L_connection_to_tun_timeout"));
		++addr_ctl.sc_unit;
	}
	_goal(mo_file_reader::gettext("L_found_virtual_card_at_slot") << ' ' << tested_card_counter);

	m_ifr_name = "utun" + std::to_string(addr_ctl.sc_unit - 1);
	return tun_fd;
}

void c_tuntap_macosx_obj::set_ipv6_address(const std::array<uint8_t, 16> &binary_address,
                                           int prefixLen) {

	_check_extern(binary_address[0] == 0xFD);
	_check_extern(binary_address[1] == 0x42);
	Wrap_NetPlatform_addAddress(m_ifr_name.c_str(), binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tuntap_macosx_obj::set_mtu(uint32_t mtu) {
	_fact("Setting MTU="<<mtu);
	const auto name = m_ifr_name.c_str();
	_fact("Setting MTU="<<mtu<<" on card: " << name);
	t_syserr error = NetPlatform_setMTU(name, mtu);
	if (error.my_code != 0)
		throw std::runtime_error("set MTU error: " + errno_to_string(error.errno_copy));
}

#endif // ANTINET_macosx
