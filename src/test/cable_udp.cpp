#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../cable/udp/cable_udp_obj.hpp"
#include "mock_c_card_selector.hpp"
#include "mock_c_asioservice_manager.hpp"


using testing::_;
using testing::Const;
using testing::Invoke;
using testing::ReturnRef;



TEST(cable_udp, constructor) {
	// create with bad c_asioservice_manager pointer
	std::shared_ptr<c_asioservice_manager_base> empty_ptr;
	mock::mock_c_card_selector card_selector;
	EXPECT_THROW(c_cable_udp cable1(empty_ptr, card_selector), std::invalid_argument);

	// create normal
	boost::asio::io_service io_service;
	c_cable_udp_addr my_addr("127.0.0.1:9000"); // should be mock TODO
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	EXPECT_NO_THROW(c_cable_udp cable2(asioservice_manage_ptr, card_selector));

	// socket open error
	mock::mock_boost_udp_socket::s_is_open = false;
	EXPECT_THROW(c_cable_udp cable3(asioservice_manage_ptr, card_selector), std::runtime_error);

	// set option error
	mock::mock_boost_udp_socket::s_is_open = true;
	mock::mock_boost_udp_socket::s_set_option_throws = true;
	EXPECT_THROW(c_cable_udp cable4(asioservice_manage_ptr, card_selector), std::runtime_error);
}
