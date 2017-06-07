#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../cable/udp/cable_udp_obj.hpp"
#include "mock_c_card_selector.hpp"
#include "mock_c_asioservice_manager.hpp"


using testing::_;
using testing::An;
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

	mock::mock_boost_udp_socket::s_is_open = true;
	mock::mock_boost_udp_socket::s_set_option_throws = false;
}

// fake address class for check bad address argument
class c_cable_bad_addr final : public c_cable_base_addr {
	public:
		c_cable_bad_addr() : c_cable_base_addr(t_cable_kind::kind_simul){}
		unique_ptr<c_cable_base_addr> clone() const override {return nullptr;}
		void print(std::ostream &) const override{}
		signed char compare_same_class(const c_cable_base_addr &) const override {return '\0';}
};

TEST(cable_udp, send_to_one_buffer) {
	// create normal
	mock::mock_c_card_selector card_selector;
	boost::asio::io_service io_service;
	c_cable_udp_addr my_addr("127.0.0.1:9000");
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	c_cable_udp cable(asioservice_manage_ptr, card_selector);

	// invalid address argument
	c_cable_bad_addr bad_addr;
	std::array<unsigned char, 1024> send_buffer;
	send_buffer.fill(0);
	EXPECT_THROW(cable.send_to(bad_addr, send_buffer.data(), send_buffer.size()), std::invalid_argument);

	// asio sent_to error
	EXPECT_CALL(cable.m_write_socket, send_to(An<const boost::asio::const_buffer &>(), An<const boost::asio::ip::udp::endpoint &>()))
		.WillOnce(Invoke(
			[] (const boost::asio::const_buffer &, const boost::asio::ip::udp::endpoint &) -> size_t {
				throw boost::system::system_error(boost::asio::error::eof);
			} // lambda
	));
	c_cable_udp_addr dst_address("192.168.0.1:9000");
	EXPECT_THROW(cable.send_to(dst_address, send_buffer.data(), send_buffer.size()), std::runtime_error);

}

TEST(cable_udp, send_to_multiple_buffers) {

}
