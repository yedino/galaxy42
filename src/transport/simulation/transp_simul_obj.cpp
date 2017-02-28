

#include "transport/simulation/transp_simul_obj.hpp"
#include "transport/simulation/world.hpp"

#include "libs1.hpp"

c_transport_simul_obj::c_transport_simul_obj(shared_ptr<c_world> world)
	:
	m_addr( world->generate_simul_transport() ) ,
	m_world( world )
{
}

c_transport_simul_obj::~c_transport_simul_obj() {
}

void c_transport_simul_obj::send_data(boost::any dest, const char *data, size_t size_of_data) {
	_check(size_of_data>=1);

	_UNUSED(dest);
	// _info("Tranport send to " << dest << " data: " << data );

	volatile char fake;
	size_t pos=0;
	pos += data[1];
	if (size_of_data>=2) pos += data[2]*256;
	if (size_of_data>=3) pos += data[2]*256*256;
	fake = data[ pos % size_of_data ]; // force a fake "write" to volatile
}



