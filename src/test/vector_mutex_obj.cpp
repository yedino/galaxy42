#include "gtest/gtest.h"
#include <iostream>
#include <exception>

#include <libs0.hpp> // for debug etc
#include <stdplus/vector_mutexed_obj.hpp>

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
	EXPECT_EQ(vec.size(), 10);

}

TEST(vector_mutexed_obj_test, push_back_test_in_threads) {
	vector_mutexed_obj<unsigned char> vec;

	std::thread t1([&](){for (size_t i : {0, 1, 2, 3, 4, 5, 6}) vec.push_back(i); });
	std::thread t2([&](){for (size_t i : {7, 8, 9, 10, 11, 12}) vec.push_back(i); });
	std::thread t3([&](){for (size_t i : {13, 14, 15, 16, 17, 18}) vec.push_back(i); });
	std::thread t4([&](){for (size_t i : {19, 20, 21, 22, 23, 24}) vec.push_back(i); });

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	std::vector<unsigned char> vec(25, 0);
	EXPECT_EQ(vec.size(), 25);
	for (unsigned char i=0; i<25; i++)
		vec.run_on_matching(
					[i](auto x){return i==x;},
					[&counter](auto x))

}

