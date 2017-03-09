#include "asio_ioservice_manager.hpp"
#include "../utils/check.hpp"

c_asioservice_manager::c_asioservice_manager(size_t size_) : m_size(size_) {
	_check(size() < capacity());
	m_ioservice_threads.resize(size_);
	for (size_t i = 0; i < m_size; i++) {
		run_ioservice(i);
	}
}

c_asioservice_manager::~c_asioservice_manager() {
	for (size_t i = 0; i < m_size; i++)
		m_ioservice_array.at(i).stop();
	for (auto & thread : m_ioservice_threads)
		thread.join();
}

void c_asioservice_manager::resize_to_at_least(size_t size_) {
	_check(size_ > m_ioservice_array.size());
	if (size_ <= m_size) return;
	for (size_t i = m_size; i < size_; i++)
		run_ioservice(i);
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

void c_asioservice_manager::run_ioservice(size_t index) {
	m_ioservice_idle_works.emplace_back(m_ioservice_array.at(index));
	m_ioservice_threads.at(index) = std::thread([&io_service_ref = m_ioservice_array.at(index)] {
		io_service_ref.run();
	});
}
