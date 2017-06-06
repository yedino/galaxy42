#include "gtest/gtest.h"
#include <iostream>
#include <exception>

#include <libs0.hpp> // for debug etc
#include <stdplus/vector_mutexed_obj.hpp>
#include <stdplus/with_mutex.hpp>

TEST(vector_mutexed_obj_test, grow_to_test_in_threads) {
	vector_mutexed_obj<unsigned char> vec;

	std::thread t1([&](){for (size_t i : {1, 3, 8, 2, 7, 2}) vec.grow_to(i); });
	std::thread t2([&](){for (size_t i : {9, 10, 2, 1, 3, 1}) vec.grow_to(i); });
	std::thread t3([&](){for (size_t i : {6, 5, 4, 3, 2, 1}) vec.grow_to(i); });
	std::thread t4([&](){for (size_t i : {0, 1, 2, 3, 4, 5}) vec.grow_to(i); });

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	EXPECT_EQ(vec.size(), 10u);

}

TEST(vector_mutexed_obj_test, push_back_test_in_threads) {
	vector_mutexed_obj<char> vec;

	std::thread t1([&](){for (size_t i : {0, 1, 2, 3, 4, 5, 6}) vec.push_back((i) ); });
	std::thread t2([&](){for (size_t i : {7, 8, 9, 10, 11, 12}) vec.push_back((i) ); });
	std::thread t3([&](){for (size_t i : {13, 14, 15, 16, 17, 18}) vec.push_back((i) ); });
	std::thread t4([&](){for (size_t i : {19, 20, 21, 22, 23, 24}) vec.push_back((i) ); });

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	std::vector<char> counters(25, 0);
	EXPECT_EQ(vec.size(), 25u);
	auto run = [&counters](char x) -> int {for (size_t i=0; i<counters.size(); i++) if (static_cast<size_t>(x)==i) counters[i]++; return 0;};
	for (size_t i=0; i<vec.size(); i++)
		vec.run_on<int, decltype(run)>(i, run); // not compile it is BAD

	for (auto i : counters) EXPECT_EQ(i, 1);
}

TEST(vector_mutexed_obj_test, run_on_matching_test) {
	//vector_mutexed_obj<stdplus::with_mutex<std::shared_timed_mutex, unsigned char> > vec;
	vector_mutexed_obj<unsigned char> vec;
	size_t size = 25;

	for (unsigned char i=0; i< size; i++)
		vec.push_back(std::move(i));
		//vec.push_back(stdplus::with_mutex<std::shared_timed_mutex,unsigned char>(i) );

	std::vector<char> counters(25, 0);
	EXPECT_EQ(vec.size(), 25u);
	auto run = [&counters](unsigned char x) -> int {counters[x]++; return 0;};
	for(int iter=0; iter<3; iter++){
	for (unsigned char i=0; i<vec.size(); i++)
	{
		auto test = [i](const unsigned char& x) -> bool {return i==x;};
		try{
			vec.run_on_matching<int, decltype(test), decltype(run)>(test, run, 1);
		}catch(const error_no_match_found& ex)
		{
			_erro(ex.what() << " Object:" << static_cast<int>(i));
		}
	}
}
	for (auto i : counters) EXPECT_EQ(i, 3);
}

TEST(vector_mutexed_obj_test, run_on_matching_exception_test) {
	vector_mutexed_obj<unsigned int> vec;
	size_t size = 10;

	for (unsigned int i=0; i< size; i++)
		vec.push_back(std::move(i%2));

	auto run = [](unsigned int x) -> unsigned int {return x;};
	auto test = [](const unsigned int& x) -> bool {return 1==x;};
	auto test2 = [](const unsigned int& x) -> bool {return 5==x;};
	auto test_case1 = [&](){vec.run_on_matching<unsigned int, decltype(test), decltype(run)>(test, run, 5);};
	auto test_case2 = [&](){vec.run_on_matching<unsigned int, decltype(test), decltype(run)>(test, run, 6);};
	auto test_case3 = [&](){vec.run_on_matching<unsigned int, decltype(test2), decltype(run)>(test2, run, 1);};
	EXPECT_NO_THROW(test_case1());
	EXPECT_THROW(test_case2(), error_not_enough_match_found);
	EXPECT_THROW(test_case3(), error_no_match_found);
}
