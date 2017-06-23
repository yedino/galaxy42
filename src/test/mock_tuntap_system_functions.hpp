#ifndef MOCK_TUNTAP_SYSTEM_FUNCTIONS_HPP
#define MOCK_TUNTAP_SYSTEM_FUNCTIONS_HPP

#include "../../depends/googletest/googlemock/include/gmock/gmock.h"
#include "../i_tuntap_system_functions.hpp"

namespace mock {

class mock_tuntap_system_functions : public i_tuntap_system_functions {
	public:
		MOCK_METHOD3(ioctl, int(int, unsigned long, void *));
		MOCK_METHOD4(NetPlatform_addAddress, t_syserr(const char *, const uint8_t *, int, int));
		MOCK_METHOD2(NetPlatform_setMTU, t_syserr(const char *, uint32_t));
};

} // namespace

#endif // MOCK_TUNTAP_SYSTEM_FUNCTIONS_HPP
