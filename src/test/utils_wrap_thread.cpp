#include "gtest/gtest.h"

#include <vector>

#include "../utils/wrap_thread.hpp"

TEST(utils_wrap_thread, thread_at) {

	std::vector<wrap_thread> vec;
	vec.resize(1);
	vec.at(0) = std::move(wrap_thread());

	EXPECT_THROW(vec.at(1) = std::move(wrap_thread()), std::out_of_range);
}

void fun0() {}
void fun1(int) {}
void fun2(int, int) {}

TEST(utils_wrap_thread, different_construct) {

	wrap_thread local_wrap_thread; // default constr
	local_wrap_thread = wrap_thread(fun0); // operator=

	wrap_thread local_wrap_thread2(fun1,34); // templ constr
	wrap_thread local_wrap_thread3(std::move(local_wrap_thread2)); // move constr
	// from now local_wrap_thread2 is empty!

}

TEST(utils_wrap_thread, empty_functions) {

	wrap_thread local_wrap_thread1(fun0);
	wrap_thread local_wrap_thread2(fun1, 3);
	wrap_thread local_wrap_thread3(fun2, 4, 7);
}

TEST(utils_wrap_thread, swap_function) {

	wrap_thread local_wrap_thread1(fun1, 3);
	wrap_thread local_wrap_thread2(fun2, 4, 7);

	std::thread::id id1 = local_wrap_thread1.get_id();

	local_wrap_thread1.swap(local_wrap_thread2); // swap

	std::thread::id id1_afterswap = local_wrap_thread2.get_id();

	EXPECT_EQ(id1,id1_afterswap);
}

TEST(utils_wrap_thread, manual_join) {

	wrap_thread local_wrap_thread1(fun1, 3);

	EXPECT_TRUE(local_wrap_thread1.joinable());

	if(local_wrap_thread1.joinable()) {
		local_wrap_thread1.join();
	}
}
