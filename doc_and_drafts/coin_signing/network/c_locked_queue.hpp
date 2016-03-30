#ifndef NETWORKLIB_LOCKEDQUEUE
#define NETWORKLIB_LOCKEDQUEUE
#include <mutex>
#include <queue>
#include <list>

// Simple mutex-guarded queue
template <typename _T>
class c_locked_queue {
private:
		std::mutex mutex;
		std::queue<_T> queue;

public:
		void push (_T value) {
			std::unique_lock<std::mutex> lock(mutex);
			queue.push(value);
		}

		_T pop () {
			std::unique_lock<std::mutex> lock(mutex);
			_T value;
			std::swap(value, queue.front());
			queue.pop();
			return value;
		}

		bool empty () {
			std::unique_lock<std::mutex> lock(mutex);
			return queue.empty();
		}
};

#endif