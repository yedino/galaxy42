#include "c_tuntap_windows.hpp"

// [todo_review_logic] needs WINAPI review

#if defined(ANTINET_windows)

#include "../../utils/check.hpp"
#include "../../c_ndp.hpp"
#include <libs0.hpp>

#include <io.h>
#ifndef NTSTATUS
	#define NTSTATUS LONG
#endif
#include <wincrypt.h>
#include <netioapi.h>
#include <ntddscsi.h>
#include <winioctl.h>

#include <type_traits>

#define TAP_CONTROL_CODE(request,method) \
	CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_MAC				TAP_CONTROL_CODE (1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION			TAP_CONTROL_CODE (2, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS		TAP_CONTROL_CODE (6, METHOD_BUFFERED)

c_is_user_admin::c_is_user_admin() {
	/**
		Routine Description: This routine returns TRUE if the caller's
		process is a member of the Administrators local group. Caller is NOT
		expected to be impersonating anyone and is expected to be able to
		open its own process and process token.
		Arguments: None.
		Return Value:
			TRUE - Caller has Administrators local group.
			FALSE - Caller does not have Administrators local group. --
	*/
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}
	if (!b) _throw_error_runtime("Administrator permissions required");
}

c_tuntap_windows_obj::c_tuntap_windows_obj()
:
	m_register_tun_path(),
	m_guid(get_device_guid()),
	m_handle(get_device_handle()),
	m_mac_address(get_mac(m_handle)),
	m_ioservice(),
	m_stream_handle(m_ioservice, m_handle)
{
	_check_sys(m_stream_handle.is_open());
}

size_t c_tuntap_windows_obj::send_to_tun(const unsigned char *data, size_t size) {
	std::array<unsigned char, 14 + 40 + 65535> output_buffer = { // eth header + ipv6 header + max ipv6 payload
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // destination MAC
		0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, // source MAC
		0x86, 0xDD // eth type: ipv6
	};
	// fill eth header
	std::copy(m_mac_address.begin(), m_mac_address.end(), output_buffer.begin()); // destination mac address
	std::copy(data, data + size, output_buffer.begin() + 14); // 14 == size of eth header
	return m_stream_handle.write_some(boost::asio::buffer(output_buffer.data(), size + 14)); // write eth frame
}

size_t c_tuntap_windows_obj::read_from_tun(unsigned char *const data, size_t size) {
	std::array<unsigned char, 14 + 40 + 65535> input_buffer;
	const unsigned char * const ipv6_begin = input_buffer.data();
	size_t readed_size = m_stream_handle.read_some(boost::asio::buffer(input_buffer.data(), input_buffer.size()));
	if (readed_size < (14 + 40)) return 0; // packet is smaller than eth header + ipv6 header, ignoring
	if (c_ndp::is_packet_neighbor_solicitation(input_buffer.data(), readed_size)) {
		const std::array<unsigned char, 94> neighbor_advertisement_packet_array = c_ndp::generate_neighbor_advertisement_new(m_mac_address, input_buffer);
		m_stream_handle.write_some(boost::asio::buffer(neighbor_advertisement_packet_array));
		return 0;
	}
	if ((input_buffer.at(22) != 0xFD) || (input_buffer.at(23) != 0x42)) return 0; // check ipv6 src address
	if ((input_buffer.at(38) != 0xFD) || (input_buffer.at(39) != 0x42)) return 0; // check ipv6 dst address
	std::copy_n(input_buffer.begin() + 14, readed_size - 14, data); // without eth header
	return readed_size - 14;
}

size_t c_tuntap_windows_obj::read_from_tun_separated_addresses(
	unsigned char * const data, size_t size,
	std::array<unsigned char, IPV6_LEN>& src_binary_address,
	std::array<unsigned char, IPV6_LEN>& dst_binary_address) {
		std::array<unsigned char, 14 + 40 + 65535> input_buffer; // eth header + ipv6 header + max ipv6 payload
		size_t readed_bytes = read_from_tun(input_buffer.data(), input_buffer.size());
		if (readed_bytes == 0) return 0;
		auto ipv6_header_begin = input_buffer.begin();
		// copy addresses, stdplus::copy_iter_and_check_no_overlap() not needed, buffers are always different
		std::copy(&input_buffer.at(8), &input_buffer.at(24), src_binary_address.begin());
		std::copy(&input_buffer.at(24), &input_buffer.at(40), dst_binary_address.begin());
		// copy content without addresses
		std::copy(&input_buffer.at(0), &input_buffer.at(8), data); // before addresses
		size_t ipv6_payload_size = readed_bytes - 40; // size of received bytes without ipv6 header
		std::copy(&input_buffer.at(40), &input_buffer.at(40 + ipv6_payload_size), data + 8);
		return readed_bytes - src_binary_address.size() - dst_binary_address.size();
}

void c_tuntap_windows_obj::async_receive_from_tun(unsigned char *const data, size_t size, const c_tuntap_base_obj::read_handler &handler) {
	_NOTREADY();
}

void c_tuntap_windows_obj::set_tun_parameters(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) {
	_fact("Setting IPv6 address, prefixLen=" << prefix_len);
	std::wstring human_name = get_human_name(m_guid);
	NET_LUID luid = get_luid(human_name);
	_fact("Setting address on human_name " << to_string(human_name));// << " luid=" << to_string(luid));

	// remove old address
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa814508(v=vs.85).aspx
	MIB_UNICASTIPADDRESS_TABLE *table = nullptr;
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa814428(v=vs.85).aspx
	NETIOAPI_API status = GetUnicastIpAddressTable(AF_INET6, &table);
	_check_sys(status == NO_ERROR);
	static_assert(std::numeric_limits<decltype(table->NumEntries)>::max() <= std::numeric_limits<size_t>::max(), "");
	for (size_t i = 0; i < static_cast<size_t>(table->NumEntries); ++i) {
		_info("Removing old addresses, i=" << i);
		if (table->Table[i].InterfaceLuid.Value == luid.Value) { // check if iface LUID == TAP LUID
			_info("Removing old addresses, entry i=" << i << " - will remove");
			// https://msdn.microsoft.com/en-us/library/windows/desktop/aa814405(v=vs.85).aspx
			if (DeleteUnicastIpAddressEntry(&table->Table[i]) != NO_ERROR) {
				FreeMibTable(table);
				throw std::runtime_error("DeleteUnicastIpAddressEntry error");
			}
		}
	}
	FreeMibTable(table);

	// set new address
	_fact("Setting new IP address");
	// https://msdn.microsoft.com/pl-pl/library/windows/desktop/aa814507(v=vs.85).aspx
	MIB_UNICASTIPADDRESS_ROW iprow;
	std::memset(&iprow, 0, sizeof(iprow));
	iprow.PrefixOrigin = IpPrefixOriginUnchanged; // The IP address prefix should be unchanged.
	iprow.SuffixOrigin = IpSuffixOriginUnchanged; // The IP address suffix should be unchanged.
	iprow.ValidLifetime = 0xFFFFFFFF; // A value of 0xffffffff is considered to be infinite.
	iprow.PreferredLifetime = 0xFFFFFFFF; // A value of 0xffffffff is considered to be infinite.
	iprow.OnLinkPrefixLength = 0xFF; // A value of 255 is commonly used to represent an illegal value.

	iprow.InterfaceLuid = luid;
	iprow.Address.si_family = AF_INET6;
	_check(sizeof(iprow.Address.Ipv6.sin6_addr) == binary_address.size()); // static_assert not works on MSVC
	std::memcpy(&iprow.Address.Ipv6.sin6_addr, binary_address.data(), binary_address.size());
	iprow.OnLinkPrefixLength = prefix_len;

	_fact("Creating unicast IP");
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa814401(v=vs.85).aspx
	status = CreateUnicastIpAddressEntry(&iprow);
	if (status != NO_ERROR) throw std::runtime_error("CreateUnicastIpAddressEntry error");
	_goal("Created unicast IP, status=" << status);
	set_mtu(mtu);
}

HANDLE c_tuntap_windows_obj::get_native_handle(){
	return m_stream_handle.native_handle();
}

// base on https://msdn.microsoft.com/en-us/library/windows/desktop/ms724256(v=vs.85).aspx
std::vector<std::wstring> c_tuntap_windows_obj::get_subkeys(HKEY hKey) {
	constexpr size_t mex_key_length = 255;
	constexpr size_t max_value_name = 16383;
	TCHAR    achKey[mex_key_length];   // buffer for subkey name
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
	TCHAR  achValue[max_value_name];
	DWORD cchValue = max_value_name;

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
			_dbg1("Add subkey " << i);
			cbName = mex_key_length;
			retCode = RegEnumKeyEx(hKey, i,
				achKey,
				&cbName,
				nullptr,
				nullptr,
				nullptr,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS) {
				// static_assert(std::is_nothrow_move_assignable<decltype(ret)::value_type>::value, ""); // not works on gcc and clang but it should never throws
				static_assert(std::is_nothrow_move_constructible<decltype(ret)::value_type>::value, "");
				ret.emplace_back(achKey); // Exception safety: strong guarantee (ISO/IEC 14882:2014(E) 23.3.6.5, std::wstring is no-throw moveable 21.4.2)
				// https://github.com/cplusplus/draft/blob/c%2B%2B14-cd/source/containers.tex#L5276
				// https://github.com/cplusplus/draft/blob/c%2B%2B14-cd/source/strings.tex#L1425
			}
		}
	}
	return ret;
}

std::wstring c_tuntap_windows_obj::get_device_guid() {
	_dbg1("strat get_device_guid");
	// Network Adapter == 4d36e972-e325-11ce-bfc1-08002be10318
	// https://msdn.microsoft.com/en-us/library/windows/hardware/ff553426(v=vs.85).aspx
	const std::wstring adapterKey = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
	_fact("Looking for device guid" << to_string(adapterKey));
	LONG status = 1;
	HKEY key = nullptr;
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, adapterKey.c_str(), 0, KEY_READ, &key);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(status));
	hkey_wrapper key_wrapped(key);
	std::vector<std::wstring> subkeys_vector;
	try {
		subkeys_vector = get_subkeys(key_wrapped.get());
	}
	catch (const std::exception &e) {
		key_wrapped.close();
		throw e;
	}
	key_wrapped.close();
	for (const auto & subkey : subkeys_vector) { // foreach sub key
		if (subkey == L"Properties") continue;
		std::wstring subkey_reg_path = adapterKey + L"\\" + subkey;
		_fact(to_string(subkey_reg_path));
		status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey_reg_path.c_str(), 0, KEY_QUERY_VALUE, &key);
		if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(status));
		// get ComponentId field
		DWORD size = 256;
		std::wstring componentId(size, '\0');
		static_assert(std::is_same<LPBYTE, unsigned char *>::value, "");
		// this reinterpret_cast is not UB(ISO/IEC 14882:2014(E) 3.10.10) because LPBYTE == unsigned char *
		// https://github.com/cplusplus/draft/blob/c%2B%2B14-cd/source/basic.tex#L4126
		// https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
		status = RegQueryValueExW(key, L"ComponentId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&componentId[0]), &size);
		if (status != ERROR_SUCCESS) {
			RegCloseKey(key);
			continue;
		}
		key_wrapped.set(key);
		std::wstring netCfgInstanceId;
		try {
			if (componentId.substr(0, 8) == L"root\\tap" || componentId.substr(0, 3) == L"tap") { // found TAP, substr throws std::out_of_range
				_note(to_string(subkey_reg_path));
				m_register_tun_path = std::move(subkey_reg_path);
				size = 256;
				netCfgInstanceId.resize(size, '\0');
				static_assert(std::is_same<LPBYTE, unsigned char *>::value, "");
				// this reinterpret_cast is not UB(ISO/IEC 14882:2014(E) 3.10.10) because LPBYTE == unsigned char *
				// https://github.com/cplusplus/draft/blob/c%2B%2B14-cd/source/basic.tex#L4126
				// https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
				status = RegQueryValueExW(key_wrapped.get(), L"NetCfgInstanceId", nullptr, nullptr, reinterpret_cast<LPBYTE>(&netCfgInstanceId[0]), &size);
				if (status != ERROR_SUCCESS) throw std::runtime_error("RegQueryValueEx error, error code " + std::to_string(GetLastError()));
				netCfgInstanceId.erase(size / sizeof(wchar_t) - 1); // remove '\0'
				_note(to_string(netCfgInstanceId));
				HANDLE handle = open_tun_device(netCfgInstanceId);
				if (handle == INVALID_HANDLE_VALUE) continue;
				else {
					BOOL ret = CloseHandle(handle);
					if (ret == 0) throw std::runtime_error("CloseHandle error, " + std::to_string(GetLastError()));
				}
				_dbg1("end of get_device_guid");
				return netCfgInstanceId;
			}
		}
		catch (const std::out_of_range &e) {
			_warn(std::string("register value processing error ") + e.what());
			_note("componentId = " + to_string(componentId));
			_note("netCfgInstanceId " + to_string(netCfgInstanceId));
		}
	}
	_erro("Can not find device in windows registry");
	throw std::runtime_error("Device not found");
}

std::wstring c_tuntap_windows_obj::get_human_name(const std::wstring &guid) {
	_dbg1("start get_human_name");
	_check_extern(!guid.empty());
	// Network Adapter == 4d36e972-e325-11ce-bfc1-08002be10318
	// https://msdn.microsoft.com/en-us/library/windows/hardware/ff553426(v=vs.85).aspx
	std::wstring connectionKey = L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	connectionKey += guid;
	connectionKey += L"\\Connection";
	_note("connectionKey " << to_string(connectionKey));
	LONG status = 1;
	HKEY key_tmp = nullptr;
	DWORD size = 256;
	std::wstring name(size, '\0');
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, connectionKey.c_str(), 0, KEY_QUERY_VALUE, &key_tmp);
	if (status != ERROR_SUCCESS) throw std::runtime_error("get_human_name: RegOpenKeyExW error");
	hkey_wrapper hkey_wrapped(key_tmp);
	// this reinterpret_cast is not UB(ISO/IEC 14882:2014(E) 3.10.10) because LPBYTE == unsigned char *
	// https://github.com/cplusplus/draft/blob/c%2B%2B14-cd/source/basic.tex#L4126
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
	status = RegQueryValueExW(hkey_wrapped.get(), L"Name", nullptr, nullptr, reinterpret_cast<LPBYTE>(&name[0]), &size);
	if (status != ERROR_SUCCESS) throw std::runtime_error("get_human_name: RegQueryValueExW error");
	name.erase(size / sizeof(wchar_t) - 1); // remove '\0'
	hkey_wrapped.close();
	return name;
}

NET_LUID c_tuntap_windows_obj::get_luid(const std::wstring &human_name) {
	_dbg1("start get_luid");
	NET_LUID ret;
	NETIO_STATUS status = ConvertInterfaceAliasToLuid(human_name.c_str(), &ret);
	if (status != ERROR_SUCCESS) throw std::runtime_error("ConvertInterfaceAliasToLuid error, error code " + std::to_string(GetLastError()));
	return ret;
}

HANDLE c_tuntap_windows_obj::get_device_handle() {
	_dbg1("start get_device_handle");
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
		BOOL ret = CloseHandle(handle);
		if (ret == 0) throw std::runtime_error("CloseHandle error, " + std::to_string(GetLastError()));
		throw std::runtime_error("DeviceIoControl error");
	}
	// set status
	int status = 1;
	unsigned long len = 0;
	bret = DeviceIoControl(handle, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), &status, sizeof(status), &len, nullptr);
	if (bret == false) {
		BOOL ret = CloseHandle(handle);
		if (ret == 0) throw std::runtime_error("CloseHandle error, " + std::to_string(GetLastError()));
		throw std::runtime_error("DeviceIoControl error");
	}
	return handle;
}

HANDLE c_tuntap_windows_obj::open_tun_device(const std::wstring &guid) noexcept {
	_dbg1("start open_tun_device");
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
	return handle;
}

std::array<uint8_t, c_tuntap_windows_obj::mac_address_size> c_tuntap_windows_obj::get_mac(HANDLE handle) {
	_dbg1("start get_mac");
	std::array<uint8_t, mac_address_size> mac_address;
	DWORD mac_size = 0;
	BOOL bret = DeviceIoControl(handle, TAP_IOCTL_GET_MAC, &mac_address.front(), mac_address.size(), &mac_address.front(), mac_address.size(), &mac_size, nullptr);
	if (bret == 0) throw std::runtime_error("DeviceIoControl error, last error " + std::to_string(GetLastError()));
	_check(mac_size == mac_address.size());
	_fact("tun device MAC address");
	for (const auto i : mac_address)
		std::cout << std::hex << static_cast<int>(i) << " ";
	std::cout << std::dec << std::endl;
	return mac_address;
}

void c_tuntap_windows_obj::set_mtu(uint32_t mtu) {
	_note("set MTU to " << mtu);
	_dbg1("register path " << to_string(m_register_tun_path));
	LONG status = 1;
	HKEY key = nullptr;
	_check(!m_register_tun_path.empty());
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, m_register_tun_path.c_str(), 0, KEY_SET_VALUE, &key);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegOpenKeyEx error, error code " + std::to_string(status));
	hkey_wrapper key_wrapped(key); // key will be closed in desctructor oh kay_wrapped
	std::wstring mtu_str_value = std::to_wstring(mtu);
	static_assert(std::is_same<BYTE, unsigned char >::value, "");
	status = RegSetValueEx(key_wrapped.get(), L"MTU", 0, REG_SZ, reinterpret_cast<const BYTE *>(mtu_str_value.c_str()), mtu_str_value.size() * sizeof(wchar_t));
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegSetValueEx error, error code " + std::to_string(status));
}

// hkey_wrapper
c_tuntap_windows_obj::hkey_wrapper::hkey_wrapper(HKEY hkey)
	:
	m_hkey(hkey),
	m_is_open(true)
{
}

c_tuntap_windows_obj::hkey_wrapper::~hkey_wrapper() {
	if (m_is_open) close();
}

HKEY &c_tuntap_windows_obj::hkey_wrapper::get() {
	if (!m_is_open) throw std::runtime_error("getting not open HKEY");
	return m_hkey;
}

void c_tuntap_windows_obj::hkey_wrapper::set(HKEY new_hkey) {
	if (m_hkey == new_hkey) return;
	if (m_is_open) close();
	m_hkey = new_hkey;
	m_is_open = true;
}

void c_tuntap_windows_obj::hkey_wrapper::close() {
	if (!m_is_open) return;
	m_is_open = false;
	auto status = RegCloseKey(m_hkey);
	if (status != ERROR_SUCCESS) throw std::runtime_error("RegCloseKey error, error code " + std::to_string(GetLastError()) + " returned value " + std::to_string(status));
}

#endif // ANTINET_windows
