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

#elif defined ANTINET_cancelio

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
