#include "asio_ioservice_manager.hpp"
#include "../utils/check.hpp"

c_asioservice_manager::c_asioservice_manager(size_t size_) : m_size(size_)
	// ,m_stop(false)
{
	guard_inv;

	_note("Starting SIOM for size=" << size_);
	guard_LOCK(m_mutex);
	_note("Starting SIOM for size=" << size_);
	_check(size() <= capacity());
	_check(size() >= 1);
	m_ioservice_threads.resize(size_);

	_note("Starting SIOM initial");
	for (size_t i = 0; i < m_size; i++) {
		run_ioservice(i);
	}
	_note("SIOM constructed");
}

c_asioservice_manager::~c_asioservice_manager() {
	_note("Destructing SIOM");
	guard_LOCK(m_mutex);
	guard_inv;

	for (size_t i = 0; i < m_size; i++) stop_ioservice(i);
	_note("Joining threads");
	for (auto & thread : m_ioservice_threads) {
		thread.join();
	}
	_note("Destruction SIOM - done");
}

void c_asioservice_manager::Precond() const {
	_check( m_size <= m_ioservice_array.size() );
	_check( m_size == m_ioservice_threads.size() );
	_check( m_size == m_ioservice_idle_works.size() );
}
void c_asioservice_manager::Postcond() const { Precond(); }

void c_asioservice_manager::resize_to_at_least(size_t size_) {
	_note("Resizing to size_" << size_ );
	guard_LOCK(m_mutex);
	guard_inv;

	_note("Resizing to size_" << size_ << " now size="<<size()<<" / capacity="<<capacity()
		<< "; other arrays: ..._threads=" << m_ioservice_threads.size() << " ...works=" << m_ioservice_idle_works.size() );

	if (size_ <= m_size) return; // we are already that big
	_check(size_ <= capacity()); // it's not allowed to resize bigger then capacity
	size_t count_new=0;
	m_ioservice_threads.resize(size_);
	for (size_t i = m_size; i < size_; i++) {
		++count_new;
		run_ioservice(i);
	}
	m_size = size_; // all this are now created
	_check( size() == size_ );
	_note("Resize doned, count_new="<<count_new);
}

size_t c_asioservice_manager::size() const {
	guard_LOCK(m_mutex);
	guard_inv;
	return m_size;
}

size_t c_asioservice_manager::capacity() const {
	guard_LOCK(m_mutex);
	guard_inv;
	return m_ioservice_array.size();
}

boost::asio::io_service &c_asioservice_manager::get_next_ioservice() {
	_mark("Requesting next SIO from SIOM");
	guard_LOCK(m_mutex);
	guard_inv;

	m_last_ioservice++;
	m_last_ioservice %= m_ioservice_array.size();

	_check(m_last_ioservice < m_ioservice_array.size());
	return m_ioservice_array.at(m_last_ioservice);
}


void c_asioservice_manager::stop_all_threadsafe() {
	guard_LOCK(m_mutex);
	guard_inv;

	_note("Stopping all in SIOM, m_size="<<m_size);
	for (size_t i = 0; i < m_size; i++) stop_ioservice(i);
}


void c_asioservice_manager::run_ioservice(size_t index) {
	guard_LOCK(m_mutex);
	guard_inv;

	// lock is already taken
	_note("Starting ioservice index="<<index<<" (from our array size=" << m_ioservice_array.size()<<")");
	m_ioservice_idle_works.emplace_back(m_ioservice_array.at(index)); // run idle work to have io service run running forever
	m_ioservice_threads.at(index) = std::thread([&io_service_ref = m_ioservice_array.at(index) , index] {
		_note("Starting in thread " << std::this_thread::get_id() << ", ioservice index="<<index
			<< " io_service_ref="<<static_cast<void*>(&io_service_ref));
		io_service_ref.run();
		_note("Ok, service started for index="<<index);
	});
}

void c_asioservice_manager::stop_ioservice(size_t index) {
	guard_LOCK(m_mutex);
	guard_inv;

	_note("Stopping ioservice index="<<index<<" (from our array size=" << m_ioservice_array.size()<<")");
	m_ioservice_array.at(index).stop();
}

