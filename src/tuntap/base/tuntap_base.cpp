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

void Wrap_NetPlatform_addAddress(const char *interfaceName,
                                 const uint8_t *address,
                                 int prefixLen,
                                 int addrFam) {

#if ( defined(__linux__) || defined(__CYGWIN__) ) || defined(__MACH__)
	_fact("Setting IP address: interfaceName="<<interfaceName
		  // <<" address="<<address // TODO print address correctly
		  <<" prefixLen="<<prefixLen
		  <<" addrFam="<<addrFam);
	t_syserr syserr = NetPlatform_addAddress(interfaceName, address, prefixLen, addrFam);
	if (syserr.my_code < 0) _throw_error_sub( tuntap_error_ip , NetPlatform_syserr_to_string(syserr) );
	_goal("IP address set as "<<address<<" prefix="<<prefixLen<<" on interface " << interfaceName << " family " << addrFam
		  << " result: " << NetPlatform_syserr_to_string(syserr));
#else
	_throw_error_runtime("You used wrapper, that is not implemented for this OS.");
#endif
}

void Wrap_NetPlatform_setMTU(const char *interfaceName, uint32_t mtu) {
#if ( defined(__linux__) || defined(__CYGWIN__) ) || defined(__MACH__)
	_fact("Setting MTU on interfaceName="<<interfaceName<<" mtu="<<mtu);
	t_syserr syserr = NetPlatform_setMTU(interfaceName, mtu);
	if (syserr.my_code < 0) _throw_error_sub( tuntap_error_mtu , NetPlatform_syserr_to_string(syserr) );
	_goal("MTU value " << mtu << " set on interface " << interfaceName
		  << " result: " << NetPlatform_syserr_to_string(syserr));
#else
	_throw_error_runtime("You used wrapper, that is not implemented for this OS.");
#endif
}

c_tuntap_base_obj::~c_tuntap_base_obj() {
	_fact("Deleting the tuntap");
}
