#include "gtest/gtest.h"
#include "../utils/check.hpp"
#include <iostream>
#include <exception>


TEST(utils_check, exception_check) {

	EXPECT_THROW({
		_check_user(false);
	}, err_check_user);

	EXPECT_THROW({
		_check_sys(false);
	}, err_check_sys);

	EXPECT_THROW({
		_check_extern(false);
	}, err_check_extern);

	EXPECT_NO_THROW({
		_check_user(true);
		_check_sys(true);
		_check_extern(true);
	});
}

TEST(utils_check, exception_try) {

	EXPECT_THROW({
		_try_user(false);
	}, err_check_user_soft);

	EXPECT_THROW({
		_try_sys(false);
	}, err_check_sys_soft);

	EXPECT_THROW({
		_try_extern(false);
	}, err_check_extern_soft);

	EXPECT_NO_THROW({
		_try_user(true);
		_try_sys(true);
		_try_extern(true);
	});
}
