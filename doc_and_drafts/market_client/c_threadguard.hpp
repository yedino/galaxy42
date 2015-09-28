#ifndef C_THREADGUARD_HPP
#define C_THREADGUARD_HPP
#include <thread>

class thread_guard {
	std::thread& t;
public:
	explicit thread_guard(std::thread& t_) :
		t(t_)
	{}
	~thread_guard() {
		if(t.joinable()) {
			t.join();
		}
	}
	thread_guard(thread_guard const&) = delete;
	thread_guard& operator=(thread_guard const&) = delete;
};

#endif // C_THREADGUARD_HPP