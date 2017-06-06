#ifndef I_TUNTAP_SYSTEM_FUNCTIONS_HPP
#define I_TUNTAP_SYSTEM_FUNCTIONS_HPP
#include <libs0.hpp>

class i_tuntap_system_functions {
	public:
		virtual int ioctl(int fd, unsigned long request, void *ifreq) = 0;
		virtual t_syserr NetPlatform_addAddress(const char* interfaceName,
		                                const uint8_t* address,
		                                int prefixLen,
		                                int addrFam) = 0;
		virtual t_syserr NetPlatform_setMTU(const char* interfaceName,
		                            uint32_t mtu) = 0;
		virtual ~i_tuntap_system_functions() = default;
};

#endif // I_TUNTAP_SYSTEM_FUNCTIONS_HPP
