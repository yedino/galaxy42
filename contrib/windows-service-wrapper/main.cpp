//sc create ServiceName binPath= absolute path to this wrapper binary
//sc delete ServiceName
#define UNICODE
#include <cwchar>
#include <functional>
#include <string>
#include <windows.h>

class c_service_wrapper final {
	public:
		/**
		 * @param child_binary_name Name of child proces binary. Wrapper and child binaries must be in the same direcrory. The call will block until service is stopped.
		 */
		static void run_as_service(const std::wstring &child_binary_name);
	private:
		static std::wstring m_name;
		static SERVICE_STATUS m_service_status;
		static SERVICE_STATUS_HANDLE m_handle_status;
		static PROCESS_INFORMATION m_process_info;

		static void service_main(long unsigned int argc, wchar_t** argv);
		static void control_handler(DWORD request);
		static void run_child_process();
};

void c_service_wrapper::run_as_service(const std::wstring &child_binary_name) {
	m_name = child_binary_name;
	SERVICE_TABLE_ENTRY m_service_table[2];
	wchar_t service_name[] = L"galaxy42_service";
	m_service_table[0].lpServiceName = service_name;
	m_service_table[0].lpServiceProc = reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(&c_service_wrapper::service_main);
	m_service_table[1].lpServiceName = nullptr;
	m_service_table[1].lpServiceProc = nullptr;
	StartServiceCtrlDispatcher(m_service_table);
}

void c_service_wrapper::service_main(long unsigned int argc, wchar_t **argv) {
	m_service_status.dwServiceType              = SERVICE_WIN32_OWN_PROCESS;
	m_service_status.dwCurrentState             = SERVICE_RUNNING; // TODO rm?
	m_service_status.dwControlsAccepted         = SERVICE_ACCEPT_STOP;
	m_service_status.dwWin32ExitCode            = NO_ERROR;
	m_service_status.dwServiceSpecificExitCode  = 0;
	m_service_status.dwCheckPoint               = 0;
	m_service_status.dwWaitHint                 = 0;

	m_handle_status = RegisterServiceCtrlHandler(L"galaxy42_service", reinterpret_cast<LPHANDLER_FUNCTION>(&c_service_wrapper::control_handler));
	if (m_handle_status == 0) return; // ERROR

	m_service_status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(m_handle_status, &m_service_status);
	while (m_service_status.dwCurrentState == SERVICE_RUNNING) {
		run_child_process();
		WaitForSingleObject(m_process_info.hProcess, INFINITE);
		CloseHandle(m_process_info.hProcess);
		CloseHandle(m_process_info.hThread);
	}
}

void c_service_wrapper::control_handler(DWORD request) {
	if (request == SERVICE_CONTROL_STOP) {
		TerminateProcess(m_process_info.hProcess, 9);
		CloseHandle(m_process_info.hProcess);
		CloseHandle(m_process_info.hThread);
		m_service_status.dwWin32ExitCode = 0;
		m_service_status.dwCurrentState  = SERVICE_STOPPED;
	}
	SetServiceStatus(m_handle_status, &m_service_status);
}

void c_service_wrapper::run_child_process() {
	// get directory path
	HMODULE hModule = GetModuleHandleW(nullptr);
	std::wstring path(MAX_PATH, '\0');
	GetModuleFileNameW(hModule, &path[0], path.size());
	auto pos = path.find_last_of(L"\\");
	path.erase(pos);
	path += L"\\";
	path += m_name;
	// running wrapped process
	STARTUPINFOW startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	ZeroMemory(&m_process_info, sizeof(m_process_info));
	CreateProcessW(&path[0], &path[0], nullptr, nullptr, false, 0, nullptr, nullptr, &startup_info, &m_process_info);
}

SERVICE_STATUS c_service_wrapper::m_service_status;
SERVICE_STATUS_HANDLE c_service_wrapper::m_handle_status;
PROCESS_INFORMATION c_service_wrapper::m_process_info;
std::wstring c_service_wrapper::m_name;

int main() {
	c_service_wrapper::run_as_service(L"tunserver.elf.exe"); // run tunserver.elf.exe as service
	return 0;
}
