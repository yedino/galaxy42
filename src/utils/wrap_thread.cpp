#include "wrap_thread.hpp"
#include "libs0.hpp"

wrap_thread::wrap_thread() noexcept
	:
	m_time_created( t_timepoint::clock::now() ), // created now
	m_time_started( t_timepoint::min() ),
	m_time_stopped( t_timepoint::min() )
{
	_note("Created thread: " << this->info());
	m_destroy_timeout=std::chrono::seconds(0);
}

wrap_thread::wrap_thread(wrap_thread &&rhs) noexcept {
	_note("Moving thread -START- from: " << rhs.info() << " onto me " << this->info() );
	if (&rhs != this) {
		if (m_thr.joinable()) {
			m_thr.join();
		}
		m_thr = std::move(rhs.m_thr);
	} else _info("Moving onto myself? Ignoring.");
	_note("Moving thread -DONE-  from: " << rhs.info() << " onto me " << this->info() );
}

wrap_thread &wrap_thread::operator=(wrap_thread &&rhs) noexcept {
	if (&rhs == this) {
		_info("Moving onto myself? Ignoring.");
		return *this;
	}
	if (m_thr.joinable()) {
		m_thr.join();
	}
	m_thr = std::move(rhs.m_thr);
	return *this;
}

bool wrap_thread::joinable() const noexcept {
	return m_thr.joinable();
}

std::thread::id wrap_thread::get_id() const noexcept {
	return m_thr.get_id();
}

void wrap_thread::join() {
	m_thr.join();
}

void wrap_thread::swap(wrap_thread &other) noexcept {
	m_thr.swap(other.m_thr);
}

wrap_thread::~wrap_thread()  {
	if(m_thr.joinable())
		m_thr.join();
}

std::string wrap_thread::info() const {
	std::stringstream ss;
	ss << m_destroy_timeout.count();
	return ss.str();
}
