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

	// normal send
	EXPECT_CALL(cable.m_write_socket, send_to(An<const boost::asio::const_buffer &>(), An<const boost::asio::ip::udp::endpoint &>()))
		.WillOnce(Invoke(
			[] (const boost::asio::const_buffer &buf, const boost::asio::ip::udp::endpoint &) -> size_t {
				return boost::asio::buffer_size(buf);
			} // lambda
	));
	EXPECT_NO_THROW(cable.send_to(dst_address, send_buffer.data(), send_buffer.size()));
}

TEST(cable_udp, send_to_multiple_buffers) {
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

	// generate buffers
	std::array<unsigned char, 1024> send_buffer, send_buffer2;
	send_buffer.fill(0); send_buffer2.fill(1);
	std::string send_buffer3(10, 10);
	std::vector<boost::asio::const_buffer> buffers;
	buffers.emplace_back(send_buffer.data(), send_buffer.size());
	buffers.emplace_back(send_buffer2.data(), send_buffer2.size());
	buffers.emplace_back(send_buffer3.data(), send_buffer3.size());

	// invalid address argument
	c_cable_bad_addr bad_addr;
	EXPECT_THROW(cable.send_to(bad_addr, buffers), std::invalid_argument);

	// asio sent_to error
	EXPECT_CALL(cable.m_write_socket, send_to(An<const std::vector< boost::asio::const_buffer> &>(), An<const boost::asio::ip::udp::endpoint &>()))
		.WillOnce(Invoke(
			[] (const std::vector< boost::asio::const_buffer> &, const boost::asio::ip::udp::endpoint &) -> size_t {
				throw boost::system::system_error(boost::asio::error::eof);
			} // lambda
	));
	c_cable_udp_addr dst_address("192.168.0.1:9000");
	EXPECT_THROW(cable.send_to(dst_address, buffers), std::runtime_error);

	// normal send
	EXPECT_CALL(cable.m_write_socket, send_to(An<const std::vector< boost::asio::const_buffer> &>(), An<const boost::asio::ip::udp::endpoint &>()))
		.WillOnce(Invoke(
			[] (const std::vector< boost::asio::const_buffer> &buffs, const boost::asio::ip::udp::endpoint &) -> size_t {
				return boost::asio::buffer_size(buffs);
			} // lambda
	));
	EXPECT_NO_THROW(cable.send_to(dst_address, buffers));
}

TEST(cable_udp, async_send_to) {
	// create normal
	mock::mock_c_card_selector card_selector;
	boost::asio::io_service io_service; // remote io_service i.e. from siom class
	c_cable_udp_addr my_addr("127.0.0.1:9000");
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	c_cable_udp cable(asioservice_manage_ptr, card_selector);
	std::array<unsigned char, 1024> send_buffer;
	send_buffer.fill(0);

	write_handler handler_normal = [&](const unsigned char *data, std::size_t size) {
		EXPECT_EQ(data, send_buffer.data());
		EXPECT_EQ(size, send_buffer.size());
	};

	write_handler handler_error = [&](const unsigned char *data, std::size_t size) {
		EXPECT_EQ(data, send_buffer.data());
		EXPECT_EQ(size, 0U);
	};
	// invalid address argument
	c_cable_bad_addr bad_addr;
	EXPECT_THROW(cable.async_send_to(bad_addr, send_buffer.data(), send_buffer.size(), handler_normal), std::invalid_argument);

	// send with error
	c_cable_udp_addr dst_address("192.168.0.1:9000");
	EXPECT_CALL(cable.m_write_socket, async_send_to(_, _, _))
		.WillOnce(Invoke([&](const boost::asio::const_buffer &,
		                     const boost::asio::ip::udp::endpoint &,
		                     std::function<void(const boost::system::error_code&, size_t)> handler) {
				boost::system::error_code error_code(boost::asio::error::eof);
				// add handler to io_service
				io_service.post([=]{handler(error_code, 0);});
		}
	));
	EXPECT_NO_THROW(cable.async_send_to(dst_address, send_buffer.data(), send_buffer.size(), handler_error));

	// send normal
	EXPECT_CALL(cable.m_write_socket, async_send_to(_, _, _))
		.WillOnce(Invoke([&](const boost::asio::const_buffer &buf,
		                     const boost::asio::ip::udp::endpoint &,
		                     std::function<void(const boost::system::error_code&, size_t)> handler) {
				// add handler to io_service
				boost::system::error_code error_code;
				io_service.post([=]{handler(error_code, boost::asio::buffer_size(buf));});
		}
	));
	EXPECT_NO_THROW(cable.async_send_to(dst_address, send_buffer.data(), send_buffer.size(), handler_normal));

	EXPECT_EQ(io_service.poll(), 2U); // execute 2 handlers
}

TEST(cable_udp, receive_from) {
	// create normal
	mock::mock_c_card_selector card_selector;
	boost::asio::io_service io_service; // remote io_service i.e. from siom class
	c_cable_udp_addr my_addr("127.0.0.1:9000");
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	c_cable_udp cable(asioservice_manage_ptr, card_selector);
	std::array<unsigned char, 1024> buffer;
	c_cable_udp_addr addr;

	// invalid address argument
	c_cable_bad_addr bad_addr;
	EXPECT_THROW(cable.receive_from(bad_addr, buffer.data(), buffer.size()), std::invalid_argument);

	// receive with error
	EXPECT_CALL(cable.m_write_socket, receive_from(_, _))
		.WillOnce(Invoke(
			[](const boost::asio::mutable_buffer &, boost::asio::ip::udp::endpoint &) ->std::size_t {
				throw boost::system::system_error(boost::asio::error::eof);
			} // lambda
	));
	EXPECT_THROW(cable.receive_from(addr, buffer.data(), buffer.size()), std::runtime_error);

	// receive normal
	boost::asio::ip::udp::endpoint expected_endpoint(boost::asio::ip::address_v4::from_string("192.168.1.1"), 9000);
	std::array<unsigned char, 1024> expected_buffer;
	expected_buffer.fill(1);
	buffer.fill(0);
	EXPECT_CALL(cable.m_write_socket, receive_from(_, _))
		.WillOnce(Invoke([&](const boost::asio::mutable_buffer &buff, boost::asio::ip::udp::endpoint &endpoint) -> std::size_t {
			endpoint = boost::asio::ip::udp::endpoint(expected_endpoint);
			boost::asio::buffer_copy(buff, boost::asio::buffer(expected_buffer.data(), expected_buffer.size()));
			return boost::asio::buffer_size(buff);
		} // lambda
	));
	EXPECT_NO_THROW(cable.receive_from(addr, buffer.data(), buffer.size()));
	EXPECT_EQ(addr.get_addr(), expected_endpoint);
	EXPECT_EQ(buffer, expected_buffer);
}


TEST(cable_udp, receive_from_selector) {
	// create normal
	mock::mock_c_card_selector card_selector;
	boost::asio::io_service io_service; // remote io_service i.e. from siom class
	c_cable_udp_addr my_addr("127.0.0.1:9000");
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	c_cable_udp cable(asioservice_manage_ptr, card_selector);
	std::array<unsigned char, 1024> buffer;

	// receive with error
	EXPECT_CALL(cable.m_write_socket, receive_from(_, _))
		.WillOnce(Invoke(
			[](const boost::asio::mutable_buffer &, boost::asio::ip::udp::endpoint &) ->std::size_t {
			throw boost::system::system_error(boost::asio::error::eof);
		} // lambda
	));
	EXPECT_THROW(cable.receive_from(card_selector, buffer.data(), buffer.size()), std::runtime_error);

	// receive normal
	boost::asio::ip::udp::endpoint expected_endpoint(boost::asio::ip::address_v4::from_string("192.168.1.1"), 9000);
	std::array<unsigned char, 1024> expected_buffer;
	expected_buffer.fill(1);
	buffer.fill(0);
	EXPECT_CALL(cable.m_write_socket, receive_from(_, _))
		.WillOnce(Invoke([&](const boost::asio::mutable_buffer &buff, boost::asio::ip::udp::endpoint &endpoint) -> std::size_t {
			endpoint = boost::asio::ip::udp::endpoint(expected_endpoint);
			boost::asio::buffer_copy(buff, boost::asio::buffer(expected_buffer.data(), expected_buffer.size()));
			return boost::asio::buffer_size(buff);
		} // lambda
	));

	EXPECT_NO_THROW(cable.receive_from(card_selector, buffer.data(), buffer.size()));
	EXPECT_EQ(buffer, expected_buffer);
	EXPECT_EQ(dynamic_cast<c_cable_udp_addr*>(card_selector.m_my_addr.get())->get_addr(), expected_endpoint);
}

TEST(cable_udp, async_receive_from) {
	// create normal
	mock::mock_c_card_selector card_selector;
	boost::asio::io_service io_service; // remote io_service i.e. from siom class
	c_cable_udp_addr my_addr("127.0.0.1:9000");
	EXPECT_CALL(Const(card_selector), get_my_addr())
		.WillRepeatedly(ReturnRef(my_addr));
	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_CALL(*dynamic_cast<mock::mock_c_asioservice_manager*>(asioservice_manage_ptr.get()), get_next_ioservice())
		.WillRepeatedly(ReturnRef(io_service));
	c_cable_udp cable(asioservice_manage_ptr, card_selector);
	std::array<unsigned char, 1024> buffer;
	buffer.fill(0);
	std::array<unsigned char, 1024> expected_buffer;
	expected_buffer.fill(1);
	boost::asio::ip::udp::endpoint expected_endpoint(boost::asio::ip::address_v4::from_string("192.168.1.1"), 9000);

	read_handler handler_normal = [&](const unsigned char *buff, std::size_t size, c_card_selector_base &selector) {
		EXPECT_EQ(buff, buffer.data());
		EXPECT_EQ(size, expected_buffer.size());
		boost::asio::ip::udp::endpoint endpoint = dynamic_cast<c_cable_udp_addr&>(selector.get_my_addr()).get_addr();
		EXPECT_EQ(endpoint, expected_endpoint);
	};

	read_handler handler_error = [&](const unsigned char *buff, std::size_t size, c_card_selector_base &selector) {
		EXPECT_EQ(buff, buffer.data());
		EXPECT_EQ(size, 0U);
		_UNUSED(selector);
	};

	// read with error
	EXPECT_CALL(cable.m_read_socket, async_receive_from(_, _, _))
		.WillOnce(Invoke(
			[&](const boost::asio::mutable_buffer &buff,
			   boost::asio::ip::udp::endpoint &endpoint,
			   std::function<void(const boost::system::error_code&, size_t)> handler) {
					_UNUSED(buff); _UNUSED(endpoint);
					boost::system::error_code error_code(boost::asio::error::eof);
					// add handler to io_service
					io_service.post([=]{handler(error_code, 0);});
			} // lambda
	));
	EXPECT_NO_THROW(cable.async_receive_from(buffer.data(), buffer.size(), handler_error));

	// read normal
	EXPECT_CALL(cable.m_read_socket, async_receive_from(_, _, _))
		.WillOnce(Invoke(
			[&](const boost::asio::mutable_buffer &buff,
			   boost::asio::ip::udp::endpoint &endpoint,
			   std::function<void(const boost::system::error_code&, size_t)> handler) {
					boost::system::error_code error_code;
					size_t bytes_transferred = boost::asio::buffer_copy(buff, boost::asio::buffer(expected_buffer.data(), expected_buffer.size()));
					endpoint = expected_endpoint;
					// add handler to io_service
					io_service.post([=]{handler(error_code, bytes_transferred);});

			} // lambda
	));

	EXPECT_NO_THROW(cable.async_receive_from(buffer.data(), buffer.size(), handler_normal));

	EXPECT_EQ(io_service.poll(), 2U); // execute 2 handlers
}
