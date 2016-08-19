// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef NETWORKLIB_LOCKEDQUEUE
#define NETWORKLIB_LOCKEDQUEUE
#if 0
#include <mutex>
#include <queue>

// Simple mutex-guarded queue
template <typename _T>
class c_locked_queue {
private:
		std::recursive_mutex mutex;
		std::queue<_T> queue;

public:
		void push (_T &&value) {
			std::unique_lock<std::recursive_mutex> lock(mutex);
			queue.push(std::forward<_T>(value));
		}

		_T pop () {
			std::unique_lock<std::recursive_mutex> lock(mutex);
			_T value;
			std::swap(value, queue.front());
			queue.pop();
			return value;
		}

		bool empty () {
			std::unique_lock<std::recursive_mutex> lock(mutex);
			return queue.empty();
		}

		std::recursive_mutex &get_mutex() {
			return mutex;
		}
};
#endif 
#endif
