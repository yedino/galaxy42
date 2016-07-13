
#if defined(_WIN32) || defined(__CYGWIN__)

#include "c_tun_device_windows.hpp"
#include <cassert>
#include <ifdef.h>
#include <io.h>
#include <netioapi.h>
#include <ntddscsi.h>
#include <winioctl.h>

c_tun_device_windows::c_tun_device_windows()
:
	m_readed_bytes(0),
	m_handle(get_device_handle()),
	m_stream_handle_ptr(std::make_unique<boost::asio::windows::stream_handle>(m_ioservice, m_handle))
{
	m_buffer.fill(0);
	assert(m_stream_handle_ptr->is_open());
	m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer), std::bind(&c_tun_device_windows::handle_read, this));
}

void c_tun_device_windows::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen) {
	MIB_UNICASTIPADDRESS_ROW iprow;
	//std::memset(&iprow, 0, sizeof(iprow));
	iprow.PrefixOrigin = IpPrefixOriginUnchanged;
	iprow.SuffixOrigin = IpSuffixOriginUnchanged;
	iprow.ValidLifetime = 0xFFFFFFFF;
	iprow.PreferredLifetime = 0xFFFFFFFF;
	iprow.OnLinkPrefixLength = 0xFF;
	auto guid = get_device_guid();
	auto human_name = get_human_name(guid);
	iprow.InterfaceLuid = get_luid(human_name);
	iprow.Address.si_family = AF_INET6;
	std::memcpy(&iprow.Address.Ipv6.sin6_addr, binary_address.data(), binary_address.size());
	iprow.OnLinkPrefixLength = prefixLen;
	auto status = CreateUnicastIpAddressEntry(&iprow);
}

bool c_tun_device_windows::incomming_message_form_tun() {
	m_ioservice.run_one();
	if (m_readed_bytes > 0) return true;
	else return false;
}

size_t c_tun_device_windows::read_from_tun(void *buf, size_t count) {
	assert(m_readed_bytes > 0);
	std::copy_n(m_buffer.begin(), m_readed_bytes, buf); // TODO!!! change base api and remove copy!!!
	size_t ret = m_readed_bytes;
	m_readed_bytes = 0;
	return ret;
}

size_t c_tun_device_windows::write_to_tun(const void *buf, size_t count) {
	boost::system::error_code ec;
	m_stream_handle_ptr->write_some(boost::asio::buffer(buf, count), ec);
	m_ioservice.run_one();
	if (ec) throw std::runtime_error("boost error " + ec.message());
}

// base on https://msdn.microsoft.com/en-us/library/windows/desktop/ms724256(v=vs.85).aspx
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
std::vector<std::wstring> get_subkeys(HKEY hKey) {
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
	DWORD    cchClassName = MAX_PATH;  // size of class string
	DWORD    cSubKeys = 0;               // number of subkeys
	DWORD    cbMaxSubKey;              // longest subkey size
	DWORD    cchMaxClass;              // longest class string
	DWORD    cValues;              // number of values for key
	DWORD    cchMaxValue;          // longest value name
	DWORD    cbMaxValueData;       // longest value data
	DWORD    cbSecurityDescriptor; // size of security descriptor
	FILETIME ftLastWriteTime;      // last write time
	DWORD i, retCode;
	std::vector<std::wstring> ret;
	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count.
	retCode = RegQueryInfoKey(
		hKey,                    // key handle
		achClass,                // buffer for class name
		&cchClassName,           // size of class string
		NULL,                    // reserved
		&cSubKeys,               // number of subkeys
		&cbMaxSubKey,            // longest subkey size
		&cchMaxClass,            // longest class string
		&cValues,                // number of values for this key
		&cchMaxValue,            // longest value name
		&cbMaxValueData,         // longest value data
		&cbSecurityDescriptor,   // security descriptor
		&ftLastWriteTime);       // last write time
								 // Enumerate the subkeys, until RegEnumKeyEx fails.
	if (cSubKeys) {
		std::cout << "Number of subkeys: " << cSubKeys << std::endl;

		for (i = 0; i < cSubKeys; i++) {
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS) {
				//std::wcout << achKey << std::endl;
				//std::cout << "get value" << std::endl;
				ret.emplace_back(std::wstring(achKey));
			}
		}
	}
	return ret;
}

std::wstring c_tun_device_windows::get_device_guid() {
	const std::wstring adapterKey = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
	LONG status = 1;
	HKEY key = nullptr;
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, adapterKey.c_str(), 0, KEY_READ, &key);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(GetLastError()));
	auto subkeys_vector = get_subkeys(key);
	RegCloseKey(key);
	for (auto & subkey : subkeys_vector) { // foreach sub key
		std::wstring subkey_reg_path = adapterKey + L"\\" + subkey;
		// std::wcout << subkey_reg_path << std::endl;
		status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey_reg_path.c_str(), 0, KEY_QUERY_VALUE, &key);
		if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(GetLastError()));
		// get ComponentId field
		DWORD size = 256;
		std::wstring componentId(size, '\0');
		status = RegQueryValueEx(key, L"ComponentId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&componentId[0]), &size);
		if (status != ERROR_SUCCESS) {
			RegCloseKey(key);
			continue;
		}
		if (componentId.substr(0, 8) == L"root\\tap") { // found TAP
			std::wcout << subkey_reg_path << std::endl;
			size = 256;
			std::wstring netCfgInstanceId(size, '\0');
			status = RegQueryValueEx(key, L"NetCfgInstanceId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&netCfgInstanceId[0]), &size);
			if (status != ERROR_SUCCESS) throw std::runtime_error("RegQueryValueEx error, error code " + std::to_string(GetLastError()));
			netCfgInstanceId.erase(size / sizeof(wchar_t) - 1); // remove '\0'
			std::wcout << netCfgInstanceId << std::endl;
			RegCloseKey(key);
			return netCfgInstanceId;
		}
		RegCloseKey(key);
	}
	throw std::runtime_error("Device not found");
}

std::wstring c_tun_device_windows::get_human_name(const std::wstring &guid) {
	assert(!guid.empty());
	std::wstring connectionKey = L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	connectionKey += guid;
	connectionKey += L"\\Connection";
	std::wcout << "connectionKey " << connectionKey << L"*******" << std::endl;
	LONG status = 1;
	HKEY key = nullptr;
	DWORD size = 256;
	std::wstring name(size, '\0');
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, connectionKey.c_str(), 0, KEY_QUERY_VALUE, &key);
	status = RegQueryValueEx(key, L"Name", nullptr, nullptr, reinterpret_cast<LPBYTE>(&name[0]), &size);
	name.erase(size / sizeof(wchar_t) - 1); // remove '\0'
	RegCloseKey(key);
	return name;
}

NET_LUID c_tun_device_windows::get_luid(const std::wstring &human_name) {
	NET_LUID ret;
	auto status = ConvertInterfaceAliasToLuid(human_name.c_str(), &ret); // TODO throw
	if (status != ERROR_SUCCESS) throw std::runtime_error("ConvertInterfaceAliasToLuid error, error code " + std::to_string(GetLastError()));
	return ret;
}


#define TAP_CONTROL_CODE(request,method) \
  CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_VERSION			TAP_CONTROL_CODE (2, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS		TAP_CONTROL_CODE (6, METHOD_BUFFERED)

HANDLE c_tun_device_windows::get_device_handle() {
	std::wstring tun_filename;
	tun_filename += L"\\\\.\\Global\\";
	tun_filename += get_device_guid();
	tun_filename += L".tap";
	BOOL bret;
	HANDLE handle = CreateFile(tun_filename.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
		0);
	if (handle == INVALID_HANDLE_VALUE) throw std::runtime_error("invalid handle");
	// get version
	ULONG version_len;
	struct {
		unsigned long major;
		unsigned long minor;
		unsigned long debug;
	} version;
	bret = DeviceIoControl(handle, TAP_IOCTL_GET_VERSION, &version, sizeof(version), &version, sizeof(version), &version_len, nullptr);
	if (bret == false) {
		CloseHandle(handle);
		throw std::runtime_error("DeviceIoControl error");
	}
	// set status
	int status = 1;
	unsigned long len = 0;
	bret = DeviceIoControl(handle, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), &status, sizeof(status), &len, nullptr);
	if (bret == false) {
		CloseHandle(handle);
		throw std::runtime_error("DeviceIoControl error");
	}
	return handle;
}

void c_tun_device_windows::handle_read(const boost::system::error_code& error, std::size_t length) {
	if (!error && length > 0) m_readed_bytes = length;
	// continue readind
	m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer), std::bind(&c_tun_device_windows::handle_read, this));
}

#endif
