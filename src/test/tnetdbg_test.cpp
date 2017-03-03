#include "gtest/gtest.h"
#include "../utils/check.hpp"
#include <iostream>
#include <exception>
#include <stdexcept>

TEST(UsePtr_abort_Test, abort_test) {

	EXPECT_EXIT(UsePtr(nullptr), ::testing::KilledBySignal(SIGABRT), "Recived abort signal");
}
