//sc create SampleService binPath= C:\cygwin\home\Tigu_WIN\test\service_wrapper.exe
//sc delete SampleService

#include <functional>
#include <string>
#include <windows.h>

class c_service_wrapper {
	public:
		c_service_wrapper(const std::wstring &child_binary_name);
	private:
		std::wstring m_name;
		SERVICE_STATUS m_service_status;
		SERVICE_STATUS_HANDLE m_handle_status;
		void service_main(int argc, char** argv);
		void control_handler(DWORD request);
		void run_child_process();
};

c_service_wrapper::c_service_wrapper(const std::wstring &child_binary_name) {
	SERVICE_TABLE_ENTRY service_table[2];
	service_table[0].lpServiceName = "galaxy42_service";
	// function pointer to service_main(int argc, char **argv)
	std::function<void(int, char**)> srv_main_fn = std::bind(&c_service_wrapper::service_main, *this, std::placeholders::_1, std::placeholders::_2); 
	service_table[0].lpServiceProc = reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(srv_main_fn.target<void(int, char**)>());
	service_table[1].lpServiceName = nullptr;
	service_table[1].lpServiceProc = nullptr;
	StartServiceCtrlDispatcher(service_table); // TODO creaye function to start
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
	m_handle_status = RegisterServiceCtrlHandler("galaxy42_service", reinterpret_cast<LPHANDLER_FUNCTION>(handler_proc_fn.target<void(DWORD)>()));
	if (m_handle_status == 0) return; // ERROR
	
	m_service_status.dwCurrentState             = SERVICE_RUNNING;
	SetServiceStatus(m_handle_status, &m_service_status);
	while (m_service_status.dwCurrentState == SERVICE_RUNNING) {
		run_child_process();
		//WaitForSingleObject( second_process, INFINITE );
		//CloseHandle( second_process );
		//CloseHandle(second_process_thread);
	}
}

void c_service_wrapper::control_handler(DWORD request) {
	
}

void c_service_wrapper::run_child_process() {
	
}

int main() {
	c_service_wrapper(L"tunserver.lef.exe");
	return 0;
}