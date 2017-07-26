
#include "utils/boost_asio_helper.hpp"

boost::asio::ip::address_v6 make_address( std::array<unsigned char,16> thebytes) {
	// noassert: size is always 16, checked --rfree
	boost::array<unsigned char,16> thebytes_boost;
	for (int i=0; i<16; ++i) thebytes_boost[i] = thebytes[i];
	boost::asio::ip::address_v6 addr(thebytes_boost);
	return addr;
}



