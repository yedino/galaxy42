#ifndef WRAP_THREAD_HPP
#define WRAP_THREAD_HPP

#include <thread>
#include <chrono>

/// More advanced then std::thread. Will join itself on destruction to avoid certain UBs. Has debug and stats.
/// @owner hb
class wrap_thread {
	public:
		wrap_thread() noexcept;

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

		std::string info() const noexcept; ///< return for debug summary of this object

		using t_clock = decltype(std::chrono::steady_clock()) ;  ///< clock I will use for my timing
		using t_timepoint = std::chrono::time_point<t_clock>; ////< timepoint I will use for my timing

		t_timepoint m_time_created; ///< when was this thread created first
		t_timepoint m_time_started; ///< when was this thread last time created/assigned
		t_timepoint m_time_stopped; ///< when was this thread stopped/joined

		bool join_if_possible(); ///< join if that is possible:w

	private:
		std::thread m_thr;
};


#endif // WRAP_THREAD_HPP
