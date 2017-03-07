#include "asio_ioservice_manager.hpp"
#include "../utils/check.hpp"

c_asioservice_manager::c_asioservice_manager(size_t size_) : m_size(size_) {
	_check(size() < capacity());
}

void c_asioservice_manager::size_at_least(size_t size_) {
	_check(size_ > m_ioservice_array.size());
	if (size_ <= m_size) return;
	m_size = size_;
}

size_t c_asioservice_manager::size() const { return m_size; }
size_t c_asioservice_manager::capacity() const { return m_ioservice_array.size(); }

boost::asio::io_service &c_asioservice_manager::get_next_ioservice() {
	m_last_ioservice++;
	m_last_ioservice %= m_ioservice_array.size();

	_check(m_last_ioservice < m_ioservice_array.size());
	return m_ioservice_array.at(m_last_ioservice);
}
