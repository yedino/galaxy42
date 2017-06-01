#include "gtest/gtest.h"
#include <stdplus/with_mutex.hpp>

TEST(with_mutex_test, get_test) {
	stdplus::with_mutex<std::shared_timed_mutex, unsigned char> obj(5);
	{
		auto ro_mutex = obj.get_lock_RO();
		EXPECT_EQ(obj.get(ro_mutex), 5);
		auto ro_mutex2 = obj.get_lock_RO();
		EXPECT_EQ(obj.get(ro_mutex2), 5);
		// obj.get(ro_mutex) = 2; // not compile it is OK
		// auto rw_mutex = obj.get_lock_RW(); // block test it is OK
	}
	{
		auto rw_mutex = obj.get_lock_RW(); // block test it is OK
		obj.get(rw_mutex) = 3;
		EXPECT_EQ(obj.get(rw_mutex), 3);
		// auto ro_mutex = obj.get_lock_RO(); // block test it is OK
	}
	{
		auto ro_mutex = obj.get_lock_RO();
		EXPECT_EQ(obj.get(ro_mutex), 3);
	}
}

TEST(with_mutex_test, constructor_test) {
	char value = 10;
	stdplus::with_mutex<std::shared_timed_mutex, char> obj(value);
	{
		auto ro_mutex = obj.get_lock_RO();
		EXPECT_EQ(obj.get(ro_mutex), value);
	}
	stdplus::with_mutex<std::shared_timed_mutex, char> obj2(std::move(obj));
	{
		auto ro_mutex = obj2.get_lock_RO();
		EXPECT_EQ(obj2.get(ro_mutex), value);
	}
	// obj = std::move(2); // not compile it is BAD
	//auto ro_mutex = obj.get_lock_RO();
	//EXPECT_EQ(obj.get(ro_mutex), 2);
}
