// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#if defined(_WIN32) || defined(__CYGWIN__)
	#define UNICODE
	#define _UNICODE
#endif

#include "c_tun_device.hpp"
#include "libs0.hpp"
#include "c_tnetdbg.hpp"
#include "../depends/cjdns-code/syserr.h"
#include "../depends/cjdns-code/NetPlatform.h"
#include <syserror_use.hpp>

#include <cstring>

/// @return error-code-netplatform as a human (developer) string (not translated, it's tech detail)
/// this translates the field t_syserr.my_code only, see other function for more
std::string NetPlatform_error_code_to_string(int err) {
	switch (err) {
		case -10: return "getaddrinfo"; break;
		case -20: return "socket_open"; break;
		case e_netplatform_err_open_fd: return "ioctl"; break; // TODO@mik
		case e_netplatform_err_ioctl: return "ioctl"; break; // TODO@mik
		case -100: return "invalid_address_family"; break;
		case -101: return "not_implemented_yet_address_family"; break;
		case -220: return "socketForIfName_socket_open"; break;
		case -230: return "socketForIfName_ioctl"; break;
		case -320: return "checkInterfaceUp_socket_open"; break;
		case -330: return "checkInterfaceUp_ioctl"; break;
		default:
			if (err<0) {
				std::ostringstream oss; oss<<"UNKNOWN_NetPlatform_ERROR("<<err<<")";
				return oss.str();
			}
	}
	return "OK";
}

/// @return error-code-netplatform as a human (developer) string (not translated, it's tech detail)
std::string NetPlatform_syserr_to_string(t_syserr syserr) {
	std::ostringstream oss;
	auto code = syserr.my_code;
	oss << "[Code=" << NetPlatform_error_code_to_string(code);
	if (code<0) oss << " errno=" << errno_to_string( syserr.errno_copy ); // was some error, show errno then
	oss << "]";
	return oss.str();
}

/**
 * Wrapper around the from-cjdns function;
 * Nicelly writes debug about this important function.
 * Errors are thrown as tuntap_error*
 */
void Wrap_NetPlatform_addAddress(const char* interfaceName,
                            const uint8_t* address,
                            int prefixLen,
                            int addrFam)
{
	#if ( defined(__linux__) || defined(__CYGWIN__) ) || defined(__MACH__)
	_fact("Setting IP address: interfaceName="<<interfaceName
		<<" address="<<address
		<<" prefixLen="<<prefixLen
		<<" addrFam="<<addrFam);
	t_syserr syserr = NetPlatform_addAddress(interfaceName, address, prefixLen, addrFam);
	if (syserr.my_code < 0) _throw_error_sub( tuntap_error_ip , NetPlatform_syserr_to_string(syserr) );
	_goal("IP address set as "<<address<<" prefix="<<prefixLen<<" on interface " << interfaceName << " family " << addrFam
		<< " result: " << NetPlatform_syserr_to_string(syserr));
	#else
		throw std::runtime_error("You used wrapper, that is not implemented for this OS.");
	#endif
}

// Wrapper around the from-cjdns function:
void Wrap_NetPlatform_setMTU(const char* interfaceName,
                        uint32_t mtu)
{
	#if ( defined(__linux__) || defined(__CYGWIN__) ) || defined(__MACH__)
	_fact("Setting MTU on interfaceName="<<interfaceName<<" mtu="<<mtu);
	t_syserr syserr = NetPlatform_setMTU(interfaceName, mtu);
	if (syserr.my_code < 0) _throw_error_sub( tuntap_error_mtu , NetPlatform_syserr_to_string(syserr) );
	_goal("MTU value " << mtu << " set on interface " << interfaceName
		<< " result: " << NetPlatform_syserr_to_string(syserr));
	#else
		throw std::runtime_error("You used wrapper, that is not implemented for this OS.");
	#endif
}



c_tun_device::c_tun_device()
 :
 m_ifr_name(""),
 m_ip6_ok(false)
{
	_note("Creating new general TUN device class");
}


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
	m_tun_fd(-1)
{
}

void c_tun_device_linux::init()
{
	const char *fd_fname = "/dev/net/tun";
	_goal("Opening TUN file (Linux driver) " << fd_fname);
	m_tun_fd = open(fd_fname, O_RDWR);
	int err = errno;
	if (m_tun_fd < 0) _throw_error_sub( tuntap_error_devtun , NetPlatform_syserr_to_string({e_netplatform_err_open_fd, err}) );
	_goal("TUN file opened as fd " << m_tun_fd);
}

void c_tun_device_linux::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen)
{
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	int err = errno;
	if (errcode_ioctl < 0) _throw_error_sub( tuntap_error_ip , NetPlatform_syserr_to_string({e_netplatform_err_ioctl, err}) );

	assert(binary_address[0] == 0xFD);
	assert(binary_address[1] == 0x42);
	_fact("Setting IP address");
	Wrap_NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefixLen, Sockaddr_AF_INET6);
	m_ifr_name = std::string(ifr.ifr_name);
	_note("Configured network IP for " << ifr.ifr_name);
	m_ip6_ok=true;
	_goal("IP address is fully configured");
}

void c_tun_device_linux::set_mtu(uint32_t mtu) {
	if (!m_ip6_ok) throw std::runtime_error("Can not set MTU - card not configured (ipv6)");
	_fact("Setting MTU" << mtu);
	const auto name = m_ifr_name.c_str();
	_fact("Setting MTU="<<mtu<<" on card: " << name);
	Wrap_NetPlatform_setMTU(name,mtu);
	_goal("MTU configured to " << mtu << " on card " << name);
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
	if (ret == -1) _throw_error( std::runtime_error("Read from tun error") );
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

size_t c_tun_device_linux::write_to_tun(void *buf, size_t count) { // TODO throw if error
	auto ret = write(m_tun_fd, buf, count);
	if (ret == -1) _throw_error( std::runtime_error("Write to tun error") );
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

//__linux__
#elif defined(_WIN32) || defined(__CYGWIN__)

#include "c_tnetdbg.hpp"
#include <boost/bind.hpp>
#include <cassert>
#include <ifdef.h>
#include <io.h>
#ifndef NTSTATUS
#define NTSTATUS LONG
#endif
#include <wincrypt.h>
#include <netioapi.h>
#include <ntddscsi.h>
#include <winioctl.h>

#define TAP_CONTROL_CODE(request,method) \
  CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_MAC				TAP_CONTROL_CODE (1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION			TAP_CONTROL_CODE (2, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS		TAP_CONTROL_CODE (6, METHOD_BUFFERED)

#if defined (__MINGW32__)
	#undef _assert
#endif

c_tun_device_windows::c_tun_device_windows()
	:
	m_guid(),
	m_ioservice(),
	m_buffer(),
	m_readed_bytes(0),
	m_handle(nullptr),
	m_stream_handle_ptr(),
	m_mac_address()
{
	_fact("Creating the windows device class (in ctor, before init)");
}


void c_tun_device_windows::init() {
	// TODO@rob move all the init stuff to here, from the init-list of constructor please so we can debug/log it
	_fact("Creating TUN/TAP (windows version)");

	m_guid = get_device_guid();
	m_handle = get_device_handle();
	m_stream_handle_ptr = std::make_unique<boost::asio::windows::stream_handle>(m_ioservice, m_handle);
	m_mac_address = get_mac(m_handle);

	m_buffer.fill(0);
	assert(m_stream_handle_ptr->is_open());
	// TODO@rob more debug
	//m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer), std::bind(&c_tun_device_windows::handle_read, this));
	m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer),
			boost::bind(&c_tun_device_windows::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void c_tun_device_windows::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen)
{
	_fact("Setting IPv6 address, prefixLen="<<prefixLen);
	auto human_name = get_human_name(m_guid);
	auto luid = get_luid(human_name);
	// _fact("Setting address on human_name ["<<human_name<<" luid="<<luid); // TODO@mik
	// remove old address
	MIB_UNICASTIPADDRESS_TABLE *table = nullptr;
	GetUnicastIpAddressTable(AF_INET6, &table);
	for (int i = 0; i < static_cast<int>(table->NumEntries); ++i) {
		_info("Removing old addresses, i="<<i);
		if (table->Table[i].InterfaceLuid.Value == luid.Value) {
		_info("Removing old addresses, entry i="<<i<<" - will remove");
			if (DeleteUnicastIpAddressEntry(&table->Table[i]) != NO_ERROR) {
				throw std::runtime_error("DeleteUnicastIpAddressEntry error");
			}
		}
	}
	FreeMibTable(table);

	// set new address
	_fact("Setting new IP address");
	MIB_UNICASTIPADDRESS_ROW iprow;
	std::memset(&iprow, 0, sizeof(iprow));
	iprow.PrefixOrigin = IpPrefixOriginUnchanged;
	iprow.SuffixOrigin = IpSuffixOriginUnchanged;
	iprow.ValidLifetime = 0xFFFFFFFF;
	iprow.PreferredLifetime = 0xFFFFFFFF;
	iprow.OnLinkPrefixLength = 0xFF;

	iprow.InterfaceLuid = luid;
	iprow.Address.si_family = AF_INET6;
	std::memcpy(&iprow.Address.Ipv6.sin6_addr, binary_address.data(), binary_address.size());
	iprow.OnLinkPrefixLength = prefixLen;

	_fact("Creating unicast IP");
	auto status = CreateUnicastIpAddressEntry(&iprow);
	_goal("Created unicast IP, status=" << status);
	// TODO check for error with status TODO@mik
	_UNUSED(status);
}

bool c_tun_device_windows::incomming_message_form_tun() {
	m_ioservice.run_one(); // <--- will call ASIO handler if there is any new data
	if (m_readed_bytes > 0) return true;
	return false;
}

size_t c_tun_device_windows::read_from_tun(void *buf, size_t count) {
	const size_t eth_offset = 10;
	m_readed_bytes -= eth_offset;
	assert(m_readed_bytes > 0);
	std::copy_n(&m_buffer[0] + eth_offset, m_readed_bytes, reinterpret_cast<uint8_t*>(buf)); // TODO!!! change base api and remove copy!!!
	size_t ret = m_readed_bytes;
	m_readed_bytes = 0;
	return ret;
}

size_t c_tun_device_windows::write_to_tun(void *buf, size_t count) {
	//std::cout << "****************write to tun" << std::endl;
	const size_t eth_header_size = 14;
	const size_t eth_offset = 4;
	std::vector<uint8_t> eth_frame(eth_header_size + count - eth_offset, 0);
	std::copy(m_mac_address.begin(), m_mac_address.end(), eth_frame.begin()); // destination mac address
	auto it = eth_frame.begin() + 6;
	// source mac address
	*it = 0xFC; ++it;
	for (int i = 0; i < 5; ++i) {
		*it = 0x00; ++it;
	}
	// eth type: ipv6
	*it = 0x86; ++it;
	*it = 0xDD; ++it;
	std::copy(reinterpret_cast<const uint8_t *>(buf) + eth_offset, reinterpret_cast<const uint8_t *>(buf) + count, it);
	boost::system::error_code ec;
	//size_t write_bytes = m_stream_handle_ptr->write_some(boost::asio::buffer(buf, count), ec); // prepares: blocks (but TUN is fast)
	size_t write_bytes = m_stream_handle_ptr->write_some(boost::asio::buffer(eth_frame), ec); // prepares: blocks (but TUN is fast)
	if (ec) throw std::runtime_error("boost error " + ec.message());
	return write_bytes;
}

// base on https://msdn.microsoft.com/en-us/library/windows/desktop/ms724256(v=vs.85).aspx
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
std::vector<std::wstring> c_tun_device_windows::get_subkeys(HKEY hKey) {
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
	DWORD retCode;
	std::vector<std::wstring> ret;
	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count.
	_fact("Query windows registry for infokeys");
	retCode = RegQueryInfoKey(
		hKey,                    // key handle
		achClass,                // buffer for class name
		&cchClassName,           // size of class string
		nullptr,                 // reserved
		&cSubKeys,               // number of subkeys
		&cbMaxSubKey,            // longest subkey size
		&cchMaxClass,            // longest class string
		&cValues,                // number of values for this key
		&cchMaxValue,            // longest value name
		&cbMaxValueData,         // longest value data
		&cbSecurityDescriptor,   // security descriptor
		&ftLastWriteTime);       // last write time
								 // Enumerate the subkeys, until RegEnumKeyEx fails.
	if (retCode != ERROR_SUCCESS) throw std::runtime_error("RegQueryInfoKey error, error code " + std::to_string(GetLastError()));
	if (cSubKeys > 0) {
		_fact("Number of subkeys: " << cSubKeys);

		for (DWORD i = 0; i < cSubKeys; i++) {
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,
				achKey,
				&cbName,
				nullptr,
				nullptr,
				nullptr,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS) {
				ret.emplace_back(std::wstring(achKey)); // Exception safety: strong guarantee (23.3.6.5, std::wstring is no-throw moveable 21.4.2)
			}
		}
	}
	return ret;
}

std::wstring c_tun_device_windows::get_device_guid() {
	const std::wstring adapterKey = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
	// _fact("Looking for device guid" << adapterKey); // TODO@mik
	LONG status = 1;
	HKEY key = nullptr; // TODO make unique_ptr
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, adapterKey.c_str(), 0, KEY_READ, &key);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(GetLastError()));
	std::vector<std::wstring> subkeys_vector;
	try {
		subkeys_vector = get_subkeys(key);
	} catch (const std::exception &e) {
		RegCloseKey(key);
		throw e;
	}
	RegCloseKey(key);
	for (const auto & subkey : subkeys_vector) { // foreach sub key
		if (subkey == L"Properties") continue;
		std::wstring subkey_reg_path = adapterKey + L"\\" + subkey;
		_fact(to_string(subkey_reg_path));
		status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey_reg_path.c_str(), 0, KEY_QUERY_VALUE, &key);
		if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(GetLastError()));
		// get ComponentId field
		DWORD size = 256;
		std::wstring componentId(size, '\0');
		 // this reinterpret_cast is not UB(3.10.10) because LPBYTE == unsigned char *
		 // https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
		status = RegQueryValueExW(key, L"ComponentId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&componentId[0]), &size);
		if (status != ERROR_SUCCESS) {
			RegCloseKey(key);
			continue;
		}
		if (componentId.substr(0, 8) == L"root\\tap" || componentId.substr(0, 3) == L"tap") { // found TAP
			_note(to_string(subkey_reg_path));
			size = 256;
			std::wstring netCfgInstanceId(size, '\0');
			// this reinterpret_cast is not UB(3.10.10) because LPBYTE == unsigned char *
			// https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
			status = RegQueryValueExW(key, L"NetCfgInstanceId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&netCfgInstanceId[0]), &size);
			if (status != ERROR_SUCCESS) throw std::runtime_error("RegQueryValueEx error, error code " + std::to_string(GetLastError()));
			netCfgInstanceId.erase(size / sizeof(wchar_t) - 1); // remove '\0'
			std::wcout << netCfgInstanceId << std::endl; // TODO new debug
			RegCloseKey(key);
			HANDLE handle = open_tun_device(netCfgInstanceId);
			if (handle == INVALID_HANDLE_VALUE) continue;
			else CloseHandle(handle);
			return netCfgInstanceId;
		}
		RegCloseKey(key);
	}
	_erro("Can not find device in windows registry");
	throw std::runtime_error("Device not found");
}

std::wstring c_tun_device_windows::get_human_name(const std::wstring &guid) {
	assert(!guid.empty());
	std::wstring connectionKey = L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	connectionKey += guid;
	connectionKey += L"\\Connection";
	std::wcout << "connectionKey " << connectionKey << std::endl;
	LONG status = 1;
	HKEY key = nullptr;
	DWORD size = 256;
	std::wstring name(size, '\0');
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, connectionKey.c_str(), 0, KEY_QUERY_VALUE, &key);
	status = RegQueryValueExW(key, L"Name", nullptr, nullptr, reinterpret_cast<LPBYTE>(&name[0]), &size);
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


HANDLE c_tun_device_windows::get_device_handle() {
	HANDLE handle = open_tun_device(m_guid);
	if (handle == INVALID_HANDLE_VALUE) throw std::runtime_error("invalid handle");
	// get version
	ULONG version_len;
	struct {
		unsigned long major;
		unsigned long minor;
		unsigned long debug;
	} version;
	BOOL bret = DeviceIoControl(handle, TAP_IOCTL_GET_VERSION, &version, sizeof(version), &version, sizeof(version), &version_len, nullptr);
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

HANDLE c_tun_device_windows::open_tun_device(const std::wstring &guid) {
	std::wstring tun_filename;
	tun_filename += L"\\\\.\\Global\\";
	tun_filename += guid;
	tun_filename += L".tap";
	BOOL bret;
	HANDLE handle = CreateFileW(tun_filename.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
		0);
	// TODO@rob check error
	return handle;
}

std::array<uint8_t, 6> c_tun_device_windows::get_mac(HANDLE handle) {
	std::array<uint8_t, 6> mac_address;
	DWORD mac_size = 0;
	BOOL bret = DeviceIoControl(handle, TAP_IOCTL_GET_MAC, &mac_address.front(), mac_address.size(), &mac_address.front(), mac_address.size(), &mac_size, nullptr);
	assert(mac_size == mac_address.size());
	for (const auto i : mac_address)
		std::cout << std::hex << static_cast<int>(i) << " ";
	std::cout << std::dec << std::endl;
	return mac_address;
}

void c_tun_device_windows::handle_read(const boost::system::error_code& error, std::size_t length) {
	//std::cout << "tun handle read" << std::endl;
	//std::cout << "readed " << length << " bytes from tun" << std::endl;

	try {
		if (error || (length < 1)) throw std::runtime_error(error.message());
		if (length < 54) throw std::runtime_error("tun data length < 54"); // 54 == sum of header sizes

		m_readed_bytes = length;
		if (c_ndp::is_packet_neighbor_solicitation(m_buffer)) {
			std::array<uint8_t, 94> neighbor_advertisement_packet = c_ndp::generate_neighbor_advertisement(m_buffer);
			boost::system::error_code ec;
			m_stream_handle_ptr->write_some(boost::asio::buffer(neighbor_advertisement_packet), ec); // prepares: blocks (but TUN is fast)
		}
	}
	catch (const std::runtime_error &e) {
		m_readed_bytes = 0;
		_erro("Problem with the TUN/TAP parser" << std::endl << e.what());
	}

	// continue reading
	m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer),
			boost::bind(&c_tun_device_windows::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
// hkey_wrapper
c_tun_device_windows::hkey_wrapper::hkey_wrapper(HKEY hkey)
:
	m_hkey(hkey)
{
}

c_tun_device_windows::hkey_wrapper::~hkey_wrapper() {
	auto status = RegCloseKey(m_hkey);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegCloseKey error, error code " + std::to_string(GetLastError()));
}

HKEY &c_tun_device_windows::hkey_wrapper::get() {
	return m_hkey;
}

// _win32 || __cygwin__

#elif defined(__MACH__)
#include "../depends/cjdns-code/NetPlatform.h"
#include "cpputils.hpp"
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
c_tun_device_apple::c_tun_device_apple() :
    m_tun_fd(get_tun_fd()),
    m_stream_handle_ptr(std::make_unique<boost::asio::posix::stream_descriptor>(m_ioservice, m_tun_fd)),
    m_buffer(),
    m_readed_bytes(0)
{
}

void c_tun_device_apple::init()
{
	_fact("Creating the MAC OS X device class (in ctor, before init)");
    m_buffer.fill(0);
    assert(m_stream_handle_ptr->is_open());
    m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer),
        [this](const boost::system::error_code &error, size_t length) {
            handle_read(error, length);
        }); // lambda
}

int c_tun_device_apple::get_tun_fd() {
    int tun_fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    int err=errno;
		if (m_tun_fd < 0) _throw_error_sub( tuntap_error_devtun , NetPlatform_syserr_to_string({e_netplatform_err_open_fd, err}) );

    // get ctl_id
    ctl_info info;
    std::memset(&info, 0, sizeof(info));
    const std::string apple_utun_control = "com.apple.net.utun_control";
    apple_utun_control.copy(info.ctl_name, apple_utun_control.size());
    if (ioctl(tun_fd,CTLIOCGINFO, &info) < 0) { // errno
	    	int err = errno;
        close(tun_fd);
				_throw_error_sub( tuntap_error_devtun , NetPlatform_syserr_to_string({e_netplatform_err_open_fd, err}) );
    }

    // connect to tun
    sockaddr_ctl addr_ctl;
    addr_ctl.sc_id = info.ctl_id;
    addr_ctl.sc_len = sizeof(addr_ctl);
    addr_ctl.sc_family = AF_SYSTEM;
    addr_ctl.ss_sysaddr = AF_SYS_CONTROL;
    addr_ctl.sc_unit = 1;
    // connect to first not used tun
    while (connect(tun_fd, reinterpret_cast<sockaddr *>(&addr_ctl), sizeof(addr_ctl)) < 0) {
        ++addr_ctl.sc_unit;
    }

    m_ifr_name = "utun" + std::to_string(addr_ctl.sc_unit - 1);
    return tun_fd;
}

void c_tun_device_apple::handle_read(const boost::system::error_code &error, size_t length) {
    if (error || (length < 1)) throw std::runtime_error(error.message());
    m_readed_bytes = length;
    // continue reading
    m_stream_handle_ptr->async_read_some(boost::asio::buffer(m_buffer),
                                         [this](const boost::system::error_code &error, size_t length) {
        handle_read(error, length);
    }); // lambda
}

void c_tun_device_apple::set_ipv6_address
        (const std::array<uint8_t, 16> &binary_address, int prefixLen) {
    assert(binary_address[0] == 0xFD);
    assert(binary_address[1] == 0x42);
    Wrap_NetPlatform_addAddress(m_ifr_name.c_str(), binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tun_device_apple::set_mtu(uint32_t mtu) {
	_fact("Setting MTU="<<mtu);
	const auto name = m_ifr_name.c_str();
	_fact("Setting MTU="<<mtu<<" on card: " << name);
  NetPlatform_setMTU(name, mtu);
}

bool c_tun_device_apple::incomming_message_form_tun() {
        m_ioservice.run_one(); // <--- will call ASIO handler if there is any new data
        if (m_readed_bytes > 0) return true;
        return false;
}

size_t c_tun_device_apple::read_from_tun(void *buf, size_t count) {
    assert(m_readed_bytes > 0);
	if(m_readed_bytes > count) throw std::runtime_error("undersized buffer");
    // TUN header
    m_buffer[0] = 0x00;
    m_buffer[1] = 0x00;
    m_buffer[2] = 0x86;
    m_buffer[3] = 0xDD;
    std::copy_n(&m_buffer[0], m_readed_bytes, reinterpret_cast<uint8_t *>(buf));
    size_t ret = m_readed_bytes;
    m_readed_bytes = 0;
    return ret;
}

size_t c_tun_device_apple::write_to_tun(void *buf, size_t count) {
    boost::system::error_code ec;
    uint8_t *buf_ptr = static_cast<uint8_t *>(buf);
    // TUN HEADER
    buf_ptr[0] = 0x00;
    buf_ptr[1] = 0x00;
    buf_ptr[2] = 0x00;
    buf_ptr[3] = 0x1E;

    size_t write_bytes = m_stream_handle_ptr->write_some(boost::asio::buffer(buf, count), ec);
    if (ec) throw std::runtime_error("boost error " + ec.message());
    return write_bytes;
}
// __MACH__
#else

c_tun_device_empty::c_tun_device_empty() { }

void c_tun_device_empty::set_ipv6_address(const std::array<uint8_t, 16> &binary_address, int prefixLen) {
	_UNUSED(binary_address);
	_UNUSED(prefixLen);
}

void c_tun_device_empty::set_mtu(uint32_t mtu) {
	_warn("Called set_mtu on empty device");
	_UNUSED(mtu);
}

bool c_tun_device_empty::incomming_message_form_tun() {
	return false;
}

size_t c_tun_device_empty::read_from_tun(void *buf, size_t count) {
	_UNUSED(buf);
	_UNUSED(count);
	return 0;
}

size_t c_tun_device_empty::write_to_tun(const void *buf, size_t count) {
	_UNUSED(buf);
	_UNUSED(count);
	return 0;
}

// else
#endif
