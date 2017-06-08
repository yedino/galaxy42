#include "gtest/gtest.h"
#include <stdplus/with_mutex.hpp>

TEST(with_mutex_test, get_test) {
	stdplus::with_mutex<MutexShared, unsigned char> obj(5);
	{
		auto &mutex = obj.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro(mutex);
		EXPECT_EQ(obj.get(lg_ro), 5);
		auto &mutex2 = obj.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro2(mutex2);
		EXPECT_EQ(obj.get(lg_ro2), 5);
		//obj.get(ro_mutex) = 2; // not compile it is OK
		//auto &mutex3 = obj.get_mutex();
		//UniqueLockGuardRW<MutexShared> lg_ro3(mutex3); // block test it is OK
	}
	{
		UniqueLockGuardRW<MutexShared> lg_rw(obj.get_mutex());
		obj.get(lg_rw) = 3;
		EXPECT_EQ(obj.get(lg_rw), 3);
		//auto &mutex2 = obj.get_mutex();
		//UniqueLockGuardRW<MutexShared> lg_ro2(mutex2); // block test it is OK
	}
	{
		auto &mutex = obj.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro(mutex);
		EXPECT_EQ(obj.get(lg_ro), 3);
	}
}

TEST(with_mutex_test, constructor_test) {
	char value = 10;
	stdplus::with_mutex<MutexShared, char> obj(value);
	{
		auto &mutex = obj.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro(mutex);
		EXPECT_EQ(obj.get(lg_ro), value);
	}
	stdplus::with_mutex<MutexShared, char> obj2(std::move(obj));
	{
		auto &mutex = obj2.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro(mutex);
		EXPECT_EQ(obj2.get(lg_ro), value);
	}
	obj = std::move(2);
	{
		auto &mutex = obj.get_mutex();
		UniqueLockGuardRO<MutexShared> lg_ro(mutex);
		EXPECT_EQ(obj.get(lg_ro), 2);
	}
}
