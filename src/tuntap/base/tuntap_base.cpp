#include "tuntap_base.hpp"

#include "libs0.hpp"

std::string NetPlatform_error_code_to_string(int err) {
	switch (err) {
	case e_netplatform_err_getaddrinfo: return "getaddrinfo"; break;
	case e_netplatform_err_open_socket: return "socket_open"; break;
	case e_netplatform_err_open_fd: return "open_fd"; break;
	case e_netplatform_err_ioctl: return "ioctl"; break;
	case e_netplatform_err_invalid_addr_family: return "invalid_address_family"; break;
	case e_netplatform_err_not_impl_addr_family: return "not_implemented_yet_address_family"; break;
	case e_netplatform_err_socketForIfName_open: return "socketForIfName_socket_open"; break;
	case e_netplatform_err_socketForIfName_ioctl: return "socketForIfName_ioctl"; break;
	case e_netplatform_err_checkInterfaceUp_open: return "checkInterfaceUp_socket_open"; break;
	case e_netplatform_err_checkInterfaceUp_ioctl: return "checkInterfaceUp_ioctl"; break;
	default:
		if (err<0) {
			std::ostringstream oss; oss<<"UNKNOWN_NetPlatform_ERROR("<<err<<")";
			return oss.str();
		}
	}
	return "OK";
}

std::string NetPlatform_syserr_to_string(t_syserr syserr) {
	std::ostringstream oss;
	auto code = syserr.my_code;
	oss << "[Code=" << NetPlatform_error_code_to_string(code);
	if (code<0) oss << " errno=" << errno_to_string( syserr.errno_copy ); // was some error, show errno then
	oss << "]";
	return oss.str();
}

c_tuntap_base_obj::~c_tuntap_base_obj() {
	_fact("Deleting the tuntap");
}
