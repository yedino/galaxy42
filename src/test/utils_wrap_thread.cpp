#include "gtest/gtest.h"

#include <chrono>

#include "../utils/wrap_thread.hpp"


TEST(utils_wrap_thread, thread_at) {

	vector<wrap_thread> vec;
	vec.resize(1);

	vec.at(0) = std::move(wrap_thread());

	EXPECT_THROW(vec.at(1) = std::move(wrap_thread()), std::out_of_range);
}

void fun0() {}
void fun1(int) {}
void fun2(int, int) {}

TEST(utils_wrap_thread, different_construct) {

	wrapped_thread local_wrap_thread; // default constr
	local_wrap_thread = wrapped_thread(fun0); // operator=

	wrapped_thread local_wrap_thread2(fun1,34); // templ constr
	wrapped_thread local_wrap_thread3(local_wrap_thread2); // move constr
	// from now local_wrap_thread2 is empty!

}

TEST(utils_wrap_thread, empty_functions) {

	wrapped_thread local_wrap_thread1(fun0);
	wrapped_thread local_wrap_thread2(fun1, 3);
	wrapped_thread local_wrap_thread3(fun2, 4, 7);
}

TEST(utils_wrap_thread, swap_function) {

	wrapped_thread local_wrap_thread1(fun1, 3);
	wrapped_thread local_wrap_thread2(fun2, 4, 7);

	std::thread::id id1 = local_wrap_thread1.get_id();

	local_wrap_thread1.swap(local_wrap_thread2); // swap

	std::thread::id id1_afterswap = local_wrap_thread2.get_id();

	EXPECT_EQ(id1,id1_afterswap);
}

TEST(utils_wrap_thread, manual_join) {

	wrapped_thread local_wrap_thread1(fun1, 3);

	using namespace std::chrono_literals;
	typedef std::chrono::duration_cast<std::chrono::duration<double>> duration;
	std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

	while (duration(std::chrono::steady_clock::now() - start).count() > 3s) {
		if (local_wrap_thread1.joinable()) {
			local_wrap_thread1.join();
			break;
		}
	}
}
