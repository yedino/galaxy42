#include "cable_shm_addr.hpp"

cable_shm_addr::cable_shm_addr(const std::string &address)
: c_cable_base_addr( boost::any() ) // filled in below
{
	init_addrdata(address);
}
