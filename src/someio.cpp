

#include "someio.hpp"


c_someio::c_someio()
: m_stop(false)
{ }

void c_someio::stop_threadsafe() {
	_info("someio sets m_stop");
	m_stop=true;
}

void c_someio::set_sockopt_timeout(t_native_socket sys_handler, std::chrono::microseconds timeout) {
	// ideas from http://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
	auto us = timeout.count();
	struct timeval timespec;
	timespec.tv_sec  = us / (1000*1000);
	timespec.tv_usec = us % (1000*1000);
	_dbg1("Setting timeout on system handler="<<sys_handler<<" to seconds " << timespec.tv_sec << " + " << timespec.tv_usec << " micro");
	int ret = setsockopt( sys_handler , SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&timespec), sizeof(timespec) );
	auto ret_errno = errno;
	if (ret != 0) {
		_warn("Can not set timeout on sys_handler="<<sys_handler<<" ret="<<ret<<" errno="<<ret_errno);
		_throw_error_runtime("Can not set timeout on sys_handler");
	}
}

std::chrono::microseconds c_someio::sockopt_timeout_get_default() const {
	return std::chrono::milliseconds(100);
}

