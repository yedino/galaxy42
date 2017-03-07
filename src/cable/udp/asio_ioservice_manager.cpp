#include "asio_ioservice_manager.hpp"
#include "../../utils/check.hpp"

size_t asio_ioservice_manager::m_last_ioservice = 0;

boost::asio::io_service &asio_ioservice_manager::get_next_ioservice() {
	m_last_ioservice++;
	m_last_ioservice %= m_ioservice_array.size();
	_check(m_last_ioservice >= m_ioservice_array.size());
	return m_ioservice_array.at(m_last_ioservice);
}
