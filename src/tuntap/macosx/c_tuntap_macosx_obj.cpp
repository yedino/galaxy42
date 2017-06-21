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
	std::array<unsigned char, 4> tun_header = {{0x00, 0x00, 0x00, 0x1E}};
	std::array<boost::asio::const_buffer, 2> buffers;
	buffers.at(0) = boost::asio::buffer(tun_header);
	buffers.at(1) = boost::asio::buffer(data, size);
	try {
		size_t written_bytes = m_tun_stream.write_some(buffers);
		if (written_bytes < tun_header.size()) throw std::runtime_error(""); // tun header not written
		return written_bytes - tun_header.size();
	} catch (const std::exception &) {
		return 0;
	}
}

size_t c_tuntap_macosx_obj::send_to_tun_separated_addresses(
		const unsigned char * const data,
		size_t size,
		const std::array<unsigned char, IPV6_LEN> &src_binary_address,
		const std::array<unsigned char, IPV6_LEN> &dst_binary_address) {
			_check_input(size >= 8);
			std::array<unsigned char, 4> tun_header = {{0x00, 0x00, 0x00, 0x1E}};
			std::array<boost::asio::const_buffer, 5> buffers;
			buffers.at(0) = boost::asio::buffer(tun_header.data(), tun_header.size());
			buffers.at(1) = boost::asio::buffer(data, 8); // version, traffic, flow label, payload length, next header, hop limit
			buffers.at(2) = boost::asio::buffer(src_binary_address.data(), src_binary_address.size());
			buffers.at(3) = boost::asio::buffer(dst_binary_address.data(), dst_binary_address.size());
			buffers.at(4) = boost::asio::buffer(data + 8, size - 8);
			try {
				size_t written_bytes = m_tun_stream.write_some(buffers);
					if (written_bytes < tun_header.size()) throw std::runtime_error(""); // tun header not written
					return written_bytes - tun_header.size();
			} catch (const std::exception &) {
				return 0;
			}
}

size_t c_tuntap_macosx_obj::read_from_tun(unsigned char * const data, size_t size) {
	std::array<boost::asio::mutable_buffer, 2> buffers;
	std::array<unsigned char, 4> tun_header;
	buffers.at(0) = boost::asio::buffer(tun_header);
	buffers.at(1) = boost::asio::buffer(data, size);
	try {
		size_t readed_bytes = m_tun_stream.read_some(buffers);
		if (readed_bytes < tun_header.size()) throw std::runtime_error("");
		return readed_bytes - tun_header.size();
	} catch (const std::exception &) {
		return 0;
	}
/*	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x86;
	data[3] = 0xDD;*/
}

size_t c_tuntap_macosx_obj::read_from_tun_separated_addresses(
		unsigned char * const data,
		size_t size, std::array<unsigned char, IPV6_LEN> &src_binary_address,
		std::array<unsigned char, IPV6_LEN> &dst_binary_address) {
			// field sizes based on rfc2460
			// https://tools.ietf.org/html/rfc2460
			_check_input(size > 8); // it must be strictly > 8, since we access 8th element below (data+8)
			std::array<unsigned char, 4> tun_header;
			std::array<boost::asio::mutable_buffer, 5> buffers;
			buffers.at(0) = boost::asio::buffer(tun_header.data(), tun_header.size());
			buffers.at(1) = boost::asio::buffer(data, 8); // version, traffic, flow label, payload length, next header, hop limit
			buffers.at(2) = boost::asio::buffer(src_binary_address.data(), src_binary_address.size());
			buffers.at(3) = boost::asio::buffer(dst_binary_address.data(), dst_binary_address.size());
			buffers.at(4) = boost::asio::buffer(data + 8, size - 8); // 8 first bytes of 'data' are filled using buffers.at(0)
			try {
				size_t readed_bytes = m_tun_stream.read_some(buffers) - src_binary_address.size() - dst_binary_address.size();
				if (readed_bytes < tun_header.size()) throw std::runtime_error("");
				return readed_bytes - tun_header.size();
			} catch (const std::runtime_error &) {
				return 0;
			}
}

void c_tuntap_macosx_obj::async_receive_from_tun(unsigned char * const data,
                                                 size_t size,
                                                 const c_tuntap_base_obj::read_handler &handler) {

	auto asio_handler = [data, handler](const boost::system::error_code& error, std::size_t bytes_transferred) {
		handler(data, bytes_transferred, error);
	};
	return m_tun_stream.async_read_some(boost::asio::buffer(data, size), asio_handler);
}

void c_tuntap_macosx_obj::set_tun_parameters(const std::array<unsigned char, IPV6_LEN> &binary_address,
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

	// ctl_info.ctl_name is a static array with size=96:
	// https://developer.apple.com/reference/kernel/kern_control.h/ctl_info
	_check_abort(apple_utun_control.size() < sizeof(info.ctl_name) );
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
	_dbg1("interface name " << m_ifr_name);
	return tun_fd;
}

void c_tuntap_macosx_obj::set_ipv6_address(const std::array<uint8_t, IPV6_LEN> &binary_address,
                                           int prefixLen) {

	_check_input(binary_address[0] == 0xFD);
	_check_input(binary_address[1] == 0x42);
	_dbg1("set ip addres for interface name " << m_ifr_name);
	Wrap_NetPlatform_addAddress(m_ifr_name.c_str(), binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tuntap_macosx_obj::set_mtu(uint32_t mtu) {
	_fact("Setting MTU="<<mtu);
	const auto name = m_ifr_name.c_str();
	_fact("Setting MTU="<<mtu<<" on card: " << name);
	t_syserr error = NetPlatform_setMTU(name, mtu);
	if (error.my_code != 0)
		_throw_error_runtime("set MTU error: " + errno_to_string(error.errno_copy));
}

#endif // ANTINET_macosx
