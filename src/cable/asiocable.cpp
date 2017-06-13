
#include "asiocable.hpp"
#include "libs1.hpp"

c_asiocable::c_asiocable(shared_ptr<c_asioservice_manager_base> &iomanager)
	:
	m_ptr_checker(iomanager), // check if iomanager != nullptr
	m_io_service( iomanager -> get_next_ioservice() ),
	m_iomanager( iomanager )
{
}

boost::asio::io_service & c_asiocable::get_io_service() noexcept {
	return m_io_service;
}

void c_asiocable::stop_threadsafe() {
	_info("Stopping cable");
	c_someio::stop_threadsafe();
	_info("Cable ASIO - will also stop his io_service");
	m_io_service.stop(); // this is thread-safe function
}

