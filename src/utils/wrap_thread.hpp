#ifndef WRAP_THREAD_HPP
#define WRAP_THREAD_HPP

#include<thread>

class wrap_thread {
public:
	wrap_thread() noexcept = default;

	template<typename Function, typename ...Args>
	explicit wrap_thread(Function&& f, Args&&... arg) {
		m_thr = std::thread(std::forward<Function>(f), std::forward<Args>(arg)...);
	}

	wrap_thread(const wrap_thread &) = delete;

	wrap_thread(wrap_thread && rhm) noexcept;

	wrap_thread & operator=(wrap_thread && rhm) noexcept;

	bool joinable() const noexcept;
	std::thread::id get_id() const noexcept;
	void join();
	void swap(wrap_thread& other) noexcept;
	~wrap_thread();
private:
	std::thread m_thr;
};


#endif // WRAP_THREAD_HPP
