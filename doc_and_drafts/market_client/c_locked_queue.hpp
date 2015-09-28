#ifndef C_LOCKED_QUEUE
#define C_LOCKED_QUEUE
#include <mutex>
#include <queue>
#include <list>

// Simple mutex-guarded queue
template<typename _T> class c_locked_queue {
  private:
	std::mutex mutex;
	std::queue<_T> queue;
  public:
	void push(_T value) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(value);
	};

	// Get top message in the queue
	// Note: not exception-safe (will lose the message)
	_T pop() {
		std::unique_lock<std::mutex> lock(mutex);
		_T value;
		std::swap(value, queue.front());
		queue.pop();
		return value;
	};

	bool empty() {
		std::unique_lock<std::mutex> lock(mutex);
		return queue.empty();
	}
};
#endif // C_LOCKED_QUEUE
