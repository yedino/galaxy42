#include "wrap_thread.hpp"
#include "libs0.hpp"

wrap_thread::wrap_thread()
	:
	m_time_created( t_timepoint::clock::now() ), // created now
	m_time_started( t_timepoint::min() ),
	m_time_stopped( t_timepoint::min() )
{
	_note("Created thread: " << this->info());
}

wrap_thread::wrap_thread(wrap_thread &&rhm) noexcept {
	_note("Moving thread -START- from: " << rhm.info() << " onto me " << this->info() );
	if (&rhm != this) {
		if (m_thr.joinable()) {
			m_thr.join();
		}
		m_thr = std::move(rhm.m_thr);
	} else _info("Moving onto myself? Ignoring.");
	_note("Moving thread -DONE-  from: " << rhm.info() << " onto me " << this->info() );
}

wrap_thread &wrap_thread::operator=(wrap_thread &&rhm) noexcept {
	if (&rhm == this) {
		_info("Moving onto myself? Ignoring.");
		return *this;
	}
	if (m_thr.joinable()) {
		m_thr.join();
	}
	m_thr = std::move(rhm.m_thr);
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
