

#include "cable/simulation/cable_simul_obj.hpp"
#include "cable/simulation/world.hpp"

#include "libs1.hpp"

/*

c_cable_simul_obj::c_cable_simul_obj(shared_ptr<c_world> world)
	:
	m_addr( world->generate_simul_cable() ) ,
	m_world( world )
{
}

c_cable_simul_obj::~c_cable_simul_obj() {
}

void c_cable_simul_obj::stop_threadsafe() {
	pfp_info("Stopping simulation card");
}

void c_cable_simul_obj::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	_check(size>=1);

	auto uuid = (dynamic_cast<const c_cable_simul_addr &>(dest)).get_addr();
	pfp_info("Tranport send to " << uuid << " data: " <<  to_debug( std::string(data , data+size) , e_debug_style_buf ) );

	volatile unsigned char fake;
	size_t pos=0;
	pos += data[1];
	if (size>=2) pos += data[2]*256;
	if (size>=3) pos += data[2]*256*256;
	fake = data[ pos % size ]; // force a fake "write" to volatile (compiler can't predict what part, it must provide entire data[]
}

void c_cable_simul_obj::async_send_to(const c_cable_base_addr &dest, const unsigned char *data, size_t size, write_handler handler) {
	pfp_UNUSED(dest); pfp_UNUSED(data); pfp_UNUSED(size); pfp_UNUSED(handler);
	_NOTREADY();
}

size_t c_cable_simul_obj::receive_from(c_cable_base_addr &source, unsigned char *const data, size_t size) {
	pfp_UNUSED(source); pfp_UNUSED(data); pfp_UNUSED(size);
	_NOTREADY();
	return 0;
}

void c_cable_simul_obj::async_receive_from(unsigned char *const data, size_t size, read_handler handler) {
	pfp_UNUSED(data); pfp_UNUSED(size); pfp_UNUSED(handler);
	_NOTREADY();
}

void c_cable_simul_obj::listen_on(c_cable_base_addr &local_address) {
	pfp_UNUSED(local_address);
	_NOTREADY();
}


*/

