#ifndef WRAP_THREAD_HPP
#define WRAP_THREAD_HPP

#include<thread>

class wrap_thread {
public:
	wrap_thread();

	template<typename Function, typename ...Args>
	explicit wrap_thread(Function&& f, Args&&... arg) {
		m_thr = std::thread(std::forward<Function>(f), std::forward<Args>(arg)...);
	}

	wrap_thread(const wrap_thread &) = delete;

	wrap_thread(wrap_thread && rhm);

	wrap_thread & operator=(wrap_thread && rhm);

	bool joinable() const;
	std::thread::id get_id() const;
	void join();
	void swap(wrap_thread& other);
	~wrap_thread();
private:
	std::thread m_thr;
};


#endif // WRAP_THREAD_HPP
