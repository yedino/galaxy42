#include "gtest/gtest.h"

#include <vector>
#include <atomic>

#include "../utils/wrap_thread.hpp"
#include "../libs0.hpp"

TEST(utils_wrap_thread, thread_at_throws_in_middle_of_move) {
	g_dbg_level_set(10,"test");

	std::vector<wrap_thread> vec;
	vec.resize(1);
	vec.at(0) = std::move(wrap_thread());

	// with normal thread, it would continue to run because it was spawned
	// in the temporary - we would have created std::thread( {some..work..in..lanbda} )
	// and the thread would keep running since the exception would end function before thread is joined
	std::atomic<bool> endflag{false};
	_mark("Thread ...");

	EXPECT_DEATH( vec.at(1) = std::move(wrap_thread(std::chrono::seconds(2), [&](){
		while(!endflag){}
	} )), ""); // catch std::abort

	endflag=true;
	EXPECT_THROW( vec.at(1) , std::out_of_range );
	EXPECT_THROW( vec.at(1).join() , std::out_of_range );
	// we would have a problem here with normal std::thread, but here it will join itself
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

TEST(utils_wrap_thread, manual_join) {

	wrap_thread local_wrap_thread1(fun1, 3);
	local_wrap_thread1.join();
}
