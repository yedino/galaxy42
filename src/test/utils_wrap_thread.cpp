#include "gtest/gtest.h"

#include <vector>
#include <atomic>

#include "../utils/wrap_thread.hpp"
#include "../libs0.hpp"


TEST(utils_wrap_thread, simple_thread_replace) {
	g_dbg_level_set(10,"test");
	// we create an empty thread and immediately replace it with another one
	wrap_thread wt;
	wt = wrap_thread();
	// wt = std::move(wrap_thread());
}

TEST(utils_wrap_thread, thread_at_throws_in_middle_of_move) {

	std::vector<wrap_thread> vec;
	vec.resize(1);
	vec.at(0) = wrap_thread();

	// with normal thread, it would continue to run because it was spawned
	// in the temporary - we would have created std::thread( {some..work..in..lanbda} )
	// and the thread would keep running since the exception would end function before thread is joined
	std::atomic<bool> endflag{false};
	_info("Thread will start now");

	auto example_function_causing_abort = [&]() {
		_info("Starting the bad function that should crash (abort)");
		try {
			auto mythread =
			 wrap_thread(std::chrono::seconds(2), [&](){
					while(!endflag) {
						int volatile x=1,y=2; x=y; // to not have thread that never makes progress
						if (x==y) { }
					}
			} );
			vec.at(1) = std::move(mythread) ;
			_erro("This code should not be reached (above the function at() should throw");
			bool joined = vec.at(1).try_join( std::chrono::seconds(5) ); // trying to join for few seconds
			_erro("This unreachable code, has joined=" << joined);
		}
		catch(const std::out_of_range &) {
			_erro("As expected, the code thrown... though we will not reach this place here, since the destructor of now detached thread will abort program.");
			FAIL(); // unreachable
		}
		_erro("Should not reach this place");
		FAIL(); // unreachable
	};

	EXPECT_DEATH( { example_function_causing_abort(); } , ""); // catch std::abort

	endflag=true;
	EXPECT_THROW( vec.at(1) , std::out_of_range );
	EXPECT_THROW(
		// get rid of ATTR_NODISCARD warn at compile time
		EXPECT_FALSE( vec.at(1).try_join(std::chrono::seconds(1)) ),
		std::out_of_range );
	// we would have a problem here with normal std::thread, but here it will join itself
}

void fun0() {
	_info("fun0");
}
void fun1(int i) {
	_info("fun1 arg: " << i);
}
void fun2(int i, int j) {
	_info("fun2 args: " << i << ',' << j);
}

TEST(utils_wrap_thread, different_construct) {

	wrap_thread local_wrap_thread; // default constr
	local_wrap_thread = wrap_thread(fun0); // operator=

	wrap_thread local_wrap_thread2(fun1,34); // templ constr
	wrap_thread local_wrap_thread3(std::move(local_wrap_thread2)); // move constr
	// from now local_wrap_thread2 is empty!

	// try_join
	EXPECT_FALSE( local_wrap_thread.try_join(std::chrono::seconds(1)) );
	EXPECT_FALSE( local_wrap_thread3.try_join(std::chrono::seconds(1)) );
}

// should not abort
TEST(utils_wrap_thread, destruct_join) {

	wrap_thread local_wrap_thread1(std::chrono::seconds(3),fun1, 3);
}

TEST(utils_wrap_thread, try_join) {

	wrap_thread local_wrap_thread1(fun1, 3);
	EXPECT_FALSE( local_wrap_thread1.try_join(std::chrono::seconds(1)) );
}

TEST(utils_wrap_thread, simple_functions) {

	wrap_thread local_wrap_thread1(fun0);
	wrap_thread local_wrap_thread2(fun1, 3);
	wrap_thread local_wrap_thread3(fun2, 4, 7);

	EXPECT_FALSE( local_wrap_thread1.try_join(std::chrono::seconds(1)) );
	EXPECT_FALSE( local_wrap_thread2.try_join(std::chrono::seconds(1)) );
	EXPECT_FALSE( local_wrap_thread3.try_join(std::chrono::seconds(1)) );
}

TEST(utils_wrap_thread, abort_not_joined) {

	EXPECT_DEATH({
		{
			wrap_thread local_wrap_thread1(fun0);
		}
	}
	, ""); // catch std::abort
}

TEST(utils_wrap_thread, crossover_timelimit) {
	g_dbg_level_set(10,"test");
	using namespace std::chrono_literals;
	EXPECT_DEATH({
		{
			wrap_thread notpossible(1s,[](){ std::this_thread::sleep_for(2s); });
		}
	}
	, ""); // catch std::abort
}

// as function so that we can use pragma macros here, not inside EXPECT_DEATH() macro
void test_utils_wrap_thread_assign_to_myself_not_possible() {
	using namespace std::chrono_literals;

			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wall"
			// "-Wself-move"

			// 1 second limit, 10 second sleep
			wrap_thread not_possible(1s,[](){ std::this_thread::sleep_for(10s); });

			not_possible = std::move(not_possible);

			#pragma GCC diagnostic pop

			wrap_thread not_possible2(1s,[](){ std::this_thread::sleep_for(10s); });
			not_possible=std::move(not_possible2);
}

TEST(utils_wrap_thread, assign_to_myself_not_possible) {
	EXPECT_DEATH({
		{
			test_utils_wrap_thread_assign_to_myself_not_possible();
		}
	}
	, ""); // catch std::abort
}

TEST(utils_wrap_thread, assign_to_myself_possible) {
	using namespace std::chrono_literals;
	wrap_thread possible(3s,[](){ std::this_thread::sleep_for(1s); });
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wall"
	possible = std::move(possible);
			#pragma GCC diagnostic pop

	wrap_thread possible2(3s,[](){ std::this_thread::sleep_for(1s); });
	possible=std::move(possible2);
}
