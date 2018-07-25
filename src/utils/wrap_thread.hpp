#ifndef WRAP_THREAD_HPP
#define WRAP_THREAD_HPP

#include "time_utils.hpp"
#include "../platform.hpp"

#include <future>
#include <thread>

using namespace time_utils;

/// More advanced then std::thread. Will join itself on destruction to avoid certain UBs. Has debug and stats.
/// @owner hb
class wrap_thread {
	public:
		/// Creates an empty-thread, that does nothing.
		/// It does not have to be joined. It does not start any thread.
		wrap_thread() noexcept;

		/// Creates an thread that must be joined by caller by using .join().
		/// If caller doesn't do that then destructor will abort/terminate the program.
		/// std::chrono::duration destroy_time = 0
		template<typename Function, typename ...Args>
		explicit wrap_thread(Function&& f, Args&&... arg)
			: wrap_thread() {

			m_time_started = t_sysclock::now();
			m_future = std::async(std::launch::async, std::forward<Function>(f), std::forward<Args>(arg)...);
		}

		/// this creates an thread that will auto-join itself in destructor (unless caller does the join himself first);
		/// In that automated join, the program will wait the specified time to be joined.
		/// If it's not joined by then, the it aborts/terminates.
		/// @param destroy_time - should be passed as std::chrono::seconds
		/// 	any other "time/chrono" variable will be interpreted as function and will run previous constructor.
		template<typename Function, typename ...Args>
		explicit wrap_thread(std::chrono::seconds destroy_time, Function&& f, Args&&... arg)
			: wrap_thread(std::forward<Function>(f), std::forward<Args>(arg)...) {

			m_destroy_timeout=destroy_time;
		}

		wrap_thread(const wrap_thread &) = delete;
		wrap_thread & operator=(const wrap_thread & rhs) = delete;

		wrap_thread(wrap_thread && rhs) noexcept;
		wrap_thread & operator=(wrap_thread && rhs) noexcept;

		/// It will wait at most around "duration" time, and if thread is not joined by then then it return false;
		/// otherwise it returns true; if thread is joined (e.g. the thread finished, or it was an empty thread).
		ATTR_NODISCARD bool try_join(std::chrono::duration<double> duration);

		~wrap_thread();

		std::string info() const; ///< return for debug summary of this object

	private:
		/// Internal function for joining in set time (m_destroy_time)
		void join();

		std::future<void> m_future;

		t_timepoint m_time_created; ///< when was this thread created first
		t_timepoint m_time_started; ///< when was this thread last time created/assigned
		t_timepoint m_time_stopped; ///< when was this thread stopped/joined
		std::chrono::seconds m_destroy_timeout;
};


#endif // WRAP_THREAD_HPP
