#include "platform.hpp"

#include "someio.hpp"

c_someio::c_someio()
: m_stop(false)
{ }

void c_someio::stop_threadsafe() {
	_info("someio sets m_stop");
	m_stop=true;
}

#ifdef ANTINET_socket_sockopt
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

#else if defined ANTINET_cancelio
void c_someio::set_sockopt_timeout(t_native_socket sys_handler, std::chrono::microseconds timeout) {
	DWORD ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
	int ret = setsockopt( sys_handler , SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms) );
	if (ret != 0) {
		int ret_errno = WSAGetLastError();
		_warn("Can not set timeout on sys_handler" << " ret=" << ret << " WSAGetLastError=" << ret_errno);
		_throw_error_runtime("Can not set timeout on sys_handler");
	}
}

void c_someio::close_all_tuntap_operations(t_native_tuntap_handler tuntap_handler) {
	_check_input(tuntap_handler != nullptr);
	// CancelIoEx doc https://msdn.microsoft.com/en-us/library/windows/desktop/aa363792(v=vs.85).aspx
	BOOL ret = CancelIoEx(tuntap_handler, nullptr);
	if (ret == 0) {
		_warn("Can not cancel tuntap operations, GetLastError=" << GetLastError());
		_throw_error_runtime("Can not cancel tuntap operations");
	}
}
#endif

std::chrono::microseconds c_someio::sockopt_timeout_get_default() const {
	return std::chrono::milliseconds(100);
}
