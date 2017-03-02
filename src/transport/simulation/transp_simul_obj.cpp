

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

void c_transport_simul_obj::send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) {
	_check(size>=1);

	auto uuid = (boost::any_cast<c_transport_simul_addr::t_addr>( dest.get_addrdata() ));
	_info("Tranport send to " << uuid << " data: " <<  to_debug( std::string(data , data+size) , e_debug_style_buf ) );

	volatile unsigned char fake;
	size_t pos=0;
	pos += data[1];
	if (size>=2) pos += data[2]*256;
	if (size>=3) pos += data[2]*256*256;
	fake = data[ pos % size ]; // force a fake "write" to volatile (compiler can't predict what part, it must provide entire data[]
}

size_t c_transport_simul_obj::receive_from(c_transport_base_addr &source, unsigned char *const data, size_t size) {
	_NOTREADY();
	return 0;
}

void c_transport_simul_obj::listen_on(c_transport_base_addr &local_address) {
	_NOTREADY();
}



