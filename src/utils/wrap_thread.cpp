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
	_note("Moving thread -START- from rhs onto me ");
	if (&rhs == this) {
		_info("Moving onto myself? Ignoring.");
	}

	join();
	m_future = std::move(rhs.m_future);
	_note("Moving thread -DONE- from rhs onto me ");
}

wrap_thread &wrap_thread::operator=(wrap_thread &&rhs) noexcept {
	_note("Moving thread -START- from rhs onto me ");
	if (&rhs == this) {
		_info("Moving onto myself? Ignoring.");
		return *this;
	}
	join();
	m_future = std::move(rhs.m_future);
	_note("Moving thread -DONE- from rhs onto me ");
	return *this;
}

void wrap_thread::join() {
	if(!m_future.valid())
		return;
	m_time_stopped = t_clock::now();
	_info(info());
	if (m_destroy_timeout == std::chrono::seconds(0)) {
		_erro("wrap_thread with not set time should be joined manualy using try_join()!");
		std::abort();
	}
	else {
		std::future_status status = m_future.wait_for(m_destroy_timeout);
		if (status != std::future_status::ready) {
			_erro("can not end thread in given time");
			std::abort();
		}
		m_future.get();
	}
}

bool wrap_thread::try_join(std::chrono::duration<double> duration) {
	std::chrono::seconds dest_time = std::chrono::duration_cast<std::chrono::seconds>(duration);

	if(!m_future.valid())
		return true;

	std::future_status status = m_future.wait_for(dest_time);
	if (status != std::future_status::ready) {
		_warn("try_join: can not end thread in given time");
		return true;
	} else {
		m_time_stopped = t_clock::now();
		_info("Successfull joined wrap_thread");
		_info(info());
		m_future.get();
		return false;
	}
	return true;
}

wrap_thread::~wrap_thread()  {

	m_time_stopped = t_clock::now();
	_info(info());

	if(!m_future.valid())
		return;

	if (m_destroy_timeout != std::chrono::seconds(0)) {
		std::future_status status = m_future.wait_for(m_destroy_timeout);
		if (status != std::future_status::ready) {
			_erro("can not end thread in given time");
			std::abort();
		}
		m_future.get();
	}
}

std::string wrap_thread::info() const {
	std::stringstream ss;
	ss << "Destroy thread timeout in sec " << m_destroy_timeout.count() << '\n';
	ss << "time created since epoch " << m_time_created.time_since_epoch().count() << '\n';
	ss << "time started since epoch " << m_time_started.time_since_epoch().count() << '\n';
	ss << "time stopped since epoch " << m_time_stopped.time_since_epoch().count() << '\n';
	return ss.str();
}
