//sc create SampleService binPath= C:\cygwin\home\Tigu_WIN\test\service_wrapper.exe
//sc delete SampleService
#define UNICODE
#include <cwchar>
#include <functional>
#include <string>
#include <windows.h>

class c_service_wrapper {
	public:
		/**
		 * @param child_binary_name Name of child proces binary. Wrapper and child binaries must be in the same direcrory.
		 */
		c_service_wrapper(const std::wstring &child_binary_name);
		void run_child(); ///< blocks
	private:
		std::wstring m_name;
		SERVICE_TABLE_ENTRY m_service_table[2];
		SERVICE_STATUS m_service_status;
		SERVICE_STATUS_HANDLE m_handle_status;
		PROCESS_INFORMATION m_process_info;

		void service_main(int argc, char** argv);
		void control_handler(DWORD request);
		void run_child_process();
};

c_service_wrapper::c_service_wrapper(const std::wstring &child_binary_name)
:
	m_name(child_binary_name)
{
	SERVICE_TABLE_ENTRY m_service_table[2];
	wchar_t service_name[] = L"galaxy42_service";
	m_service_table[0].lpServiceName = service_name;
	// function pointer to service_main(int argc, char **argv)
	std::function<void(int, char**)> srv_main_fn = std::bind(&c_service_wrapper::service_main, *this, std::placeholders::_1, std::placeholders::_2);
	m_service_table[0].lpServiceProc = reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(srv_main_fn.target<void(int, char**)>());
	m_service_table[1].lpServiceName = nullptr;
	m_service_table[1].lpServiceProc = nullptr;
}

void c_service_wrapper::run_child() {
	StartServiceCtrlDispatcher(m_service_table); // TODO create function to start
}

void c_service_wrapper::service_main(int argc, char **argv) {
	m_service_status.dwServiceType              = SERVICE_WIN32_OWN_PROCESS;
	m_service_status.dwCurrentState             = SERVICE_RUNNING; // TODO rm?
	m_service_status.dwControlsAccepted         = SERVICE_ACCEPT_STOP;
	m_service_status.dwWin32ExitCode            = NO_ERROR;
	m_service_status.dwServiceSpecificExitCode  = 0;
	m_service_status.dwCheckPoint               = 0;
	m_service_status.dwWaitHint                 = 0;

	// function pointer to control_handler(DWORD request)
	std::function<void(DWORD)> handler_proc_fn = std::bind(&c_service_wrapper::control_handler, *this, std::placeholders::_1);
	m_handle_status = RegisterServiceCtrlHandler(L"galaxy42_service", reinterpret_cast<LPHANDLER_FUNCTION>(handler_proc_fn.target<void(DWORD)>()));
	if (m_handle_status == 0) return; // ERROR

	m_service_status.dwCurrentState             = SERVICE_RUNNING;
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
	// running process
	STARTUPINFOW startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	ZeroMemory(&m_process_info, sizeof(m_process_info));
	auto ret = CreateProcessW(&path[0], &path[0], nullptr, nullptr, false, 0, nullptr, nullptr, &startup_info, &m_process_info);
	if (!ret) return;
}

int main() {
	c_service_wrapper service_wrapper(L"tunserver.elf.exe");
	service_wrapper.run_child();
	return 0;
}
