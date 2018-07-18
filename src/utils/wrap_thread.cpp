#include "wrap_thread.hpp"
#include <tnetdbg.hpp>


wrap_thread::wrap_thread() noexcept
	:
	m_time_created( t_timepoint::clock::now() ), // created now
	m_time_started( t_timepoint::min() ),
	m_time_stopped( t_timepoint::min() )
{
	m_destroy_timeout=std::chrono::seconds(0);
	pfp_note("Created thread: " << this->info());
}

wrap_thread::wrap_thread(wrap_thread &&rhs) noexcept {
	pfp_note("Moving thread -START- from rhs onto me ");

	join();
	m_future = std::move(rhs.m_future);
	pfp_note("Moving thread -DONE- from rhs onto me ");
}

wrap_thread &wrap_thread::operator=(wrap_thread &&rhs) noexcept {
	pfp_note("Moving thread -START- from rhs onto me ");
	if (&rhs == this) {
		pfp_info("Moving onto myself? Ignoring.");
		return *this;
	}
	join();
	m_future = std::move(rhs.m_future);
	pfp_note("Moving thread -DONE- from rhs onto me ");
	return *this;
}

void wrap_thread::join() {
	if(!m_future.valid())
		return;
	if (m_destroy_timeout == std::chrono::seconds(0)) {
		pfp_erro("wrap_thread with not set time should be joined manualy using try_join()!");
		std::abort();
	}
	else {
		std::future_status status = m_future.wait_for(m_destroy_timeout);
		if (status != std::future_status::ready) {
			pfp_erro("can not end thread in given time");
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
		pfp_warn("try_join: can not end thread in given time");
		return true;
	} else {
		m_time_stopped = t_sysclock::now();
		pfp_info("Successfully joined wrap_thread");
		m_future.get();
		return false;
	}
	return true;
}

wrap_thread::~wrap_thread()  {
	m_time_stopped = t_sysclock::now();
	pfp_info("wrap_thread destructor." + info());

	if(!m_future.valid()) {
		pfp_info("wrap_thread destructor - nothing to do (it was joined already?)"); // XXX more comments
		return;
	}

	if (m_destroy_timeout != std::chrono::seconds(0)) {
		pfp_info("wrap_thread destructor - will try to join it automatically, for time: " << m_destroy_timeout.count());
		std::future_status status = m_future.wait_for(m_destroy_timeout);
		if (status != std::future_status::ready) {
			pfp_erro("can not end thread in given time. Will abort now.");
			std::abort();
		}
		pfp_info("thread was correctly joined it seems");
		m_future.get(); // why? XXX
	} else {
		pfp_erro("This thread was not joined, and you set to not wait for joining. Will abort.");
		std::abort();
	}
}


std::string wrap_thread::info() const {

	std::stringstream ss;
	if(m_destroy_timeout != std::chrono::seconds(0)) {
		ss << "Destroy thread timeout in sec " << m_destroy_timeout.count() << '\n';
	} else {
		ss << "Destroy thread timeout not set\n";
	}
	if(m_time_created != t_timepoint::min())
		ss << "time created: " << timepoint_to_readable(m_time_created) << '\n';
	if(m_time_started != t_timepoint::min()) {
		ss << "time started: " << timepoint_to_readable(m_time_started) << '\n';
		ss << "time stopped: " << timepoint_to_readable(m_time_stopped) << '\n';
	} else
		ss << "thread was not started\n";

	return ss.str();
}
