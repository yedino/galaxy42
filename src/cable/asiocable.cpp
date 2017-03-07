
#include "asiocable.hpp"
#include "libs1.hpp"

c_asiocable::c_asiocable(shared_ptr< c_asioservice_manager > & iomanager)
	:
	m_io_service( iomanager -> get_next_ioservice() ),
	m_iomanager( iomanager )
{
}

