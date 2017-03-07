
#include "asiocable.hpp"
#include "libs1.hpp"

c_asiocable::c_asiocable(shared_ptr< asio_ioservice_manager > ioservice_manager_)
	:
	m_io_service( ioservice_manager_ -> get_next_ioservice() ),
	m_ioservice_manager( ioservice_manager_ )
{
}

