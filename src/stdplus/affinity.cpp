
#if stdplus_platform_linux
	#define _GNU_SOURCE
	#include <pthread.h>
	#include <errno.h>
#endif

#include <thread>
#include <string>
#include <sstream>
#include <exception>

#include <string.h>

#include <stdplus/platform.hpp>

namespace stdplus {

namespace affinity {


#if defined(stdplus_platform_windows)
#include <windows.h>
#endif
void set_current_thread_affinity(int cpu_nr) {

	if ( cpu_nr == -1 ) return ; // no affinity

#if defined(stdplus_platform_linux)

	if ( (cpu_nr < 0) || (cpu_nr >= CPU_SETSIZE ) ) {
		std::ostringstream oss;
		oss << "Invalid cpu_nr=" << cpu_nr << ".";
		throw std::runtime_error( oss.str() );
	}
	// --- cpu_nr is valid for CPU_SET etc ---

	cpu_set_t cpuset;
	CPU_ZERO( &cpuset );
	CPU_SET( cpu_nr , &cpuset );

	int result = pthread_setaffinity_np( pthread_self() , sizeof(cpu_set_t), &cpuset);
	if (result != 0) {
		std::ostringstream oss;
		oss << "Error while setting affinity cpu_nr=" << cpu_nr << ": ";
		oss << result << " = " << strerror(result) ;
		oss << ".";
		throw std::runtime_error( oss.str() );
	}

#elif defined(stdplus_platform_windows)
	DWORD mask = 0;
	if (cpu_nr > std::thread::hardware_concurrency()) throw std::invalid_argument("cpu nr is too big");
	// On a system with more than 64 processors, the affinity mask must specify processors in the thread's current processor group. 
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms686247(v=vs.85).aspx
	if (cpu_nr > 64) throw std::invalid_argument("cpu nr > 64 is not supported");
	mask = 1 << (cpu_nr-1);
	HANDLE this_thread_handle = GetCurrentThread();
	if (SetThreadAffinityMask(this_thread_handle, mask) == 0)
		throw std::runtime_error("SetThreadAffinityMask error, GetLastError = " + std::to_string(GetLastError()));

#elif defined(stdplus_platform_macosx)

	throw std::runtime_error("Setting thread cpu affinity not supported on this platform currently.");

#else

	throw std::runtime_error("Setting thread cpu affinity not supported on this platform currently.");

#endif

}



}

}



