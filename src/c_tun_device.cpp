// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#include "c_tun_device.hpp"

#ifdef __linux__
#include <cassert>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "c_tnetdbg.hpp"
#include "../depends/cjdns-code/NetPlatform.h"
#include "cpputils.hpp"
c_tun_device_linux::c_tun_device_linux()
:
	m_tun_fd(open("/dev/net/tun", O_RDWR))
{
	assert(! (m_tun_fd<0) ); // TODO throw?
}

void c_tun_device_linux::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen) {
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	if (errcode_ioctl < 0) throw std::runtime_error("ioctl error");
	assert(binary_address[0] == 0xFD);
	assert(binary_address[1] == 0x42);
	NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tun_device_linux::set_mtu(uint32_t mtu) {
	_UNUSED(mtu);
	_NOTREADY();
}

bool c_tun_device_linux::incomming_message_form_tun() {
	fd_set fd_set_data;
	FD_ZERO(&fd_set_data);
	FD_SET(m_tun_fd, &fd_set_data);
	timeval timeout { 0 , 500 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html
	auto select_result = select( m_tun_fd+1, &fd_set_data, nullptr, nullptr, & timeout); // <--- blocks
	_assert(select_result >= 0);
	if (FD_ISSET(m_tun_fd, &fd_set_data)) return true;
	else return false;
}

size_t c_tun_device_linux::read_from_tun(void *buf, size_t count) { // TODO throw if error
	ssize_t ret = read(m_tun_fd, buf, count); // <-- read data from TUN
	if (ret == -1) throw std::runtime_error("Read from tun error");
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

size_t c_tun_device_linux::write_to_tun(const void *buf, size_t count) { // TODO throw if error
	auto ret = write(m_tun_fd, buf, count);
	if (ret == -1) throw std::runtime_error("Write to tun error");
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

#endif // __linux__



#if defined(_WIN32) || defined(__CYGWIN__)

#include "netioapi.h"

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

#endif