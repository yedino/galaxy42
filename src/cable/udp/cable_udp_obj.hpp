#pragma once
#ifndef cable_UDP_OBJ_HPP
#define cable_UDP_OBJ_HPP

#include "../base/cable_base_obj.hpp"
#include <boost/asio.hpp>
#include <list>
#include <mutex>
#include "libs0.hpp"
#include "../asiocable.hpp"
#include "../asio_ioservice_manager.hpp"
#include "../../test/mock_boost_udp_socket.hpp"
#include "../../test/mock_c_card_selector.hpp"

#include <cable/selector.hpp>
#include <cable/udp/cable_udp_addr.hpp>

enum class t_multisocket_kind {
	one_rw=1, ///< one socket: for both read and write (then we do not need to e.g. do SO_REUSEPORT)
	separate_rw, ///< two sockets: separate read, and separate write sockets, probably we need SO_REUSEPORT
};

class c_cable_udp final : public c_asiocable {
	FRIEND_TEST(cable_udp, constructor);
	FRIEND_TEST(cable_udp, send_to_one_buffer);
	FRIEND_TEST(cable_udp, send_to_multiple_buffers);
	FRIEND_TEST(cable_udp, async_send_to);
	FRIEND_TEST(cable_udp, receive_from);
	FRIEND_TEST(cable_udp, receive_from_selector);
	FRIEND_TEST(cable_udp, async_receive_from);
	#ifdef USE_MOCK
		using t_socket_type = mock::mock_boost_udp_socket;
		using t_selector_type = mock::mock_c_card_selector;
	#else
		using t_socket_type = boost::asio::ip::udp::socket;
		using t_selector_type = c_card_selector;
	#endif
	public:
		/**
		 * create UDP cable to recv/send, using one source address (can be just port)
		 * @param iomanager - use this manager
		 * @param source_addr - we will send from this source address. Can be general ANY address e.g. 0.0.0.0:9042(?)
		 * to enforce port, or a specific one to enforce entire source address.
		 * @throws std::runtime_error on failure
		 */
		c_cable_udp(shared_ptr<c_asioservice_manager_base> & iomanager, const c_card_selector_base &source_addr);

		void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) override;
		void send_to(const c_cable_base_addr & dest, const t_asio_buffers_send & buffers) override;
		void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler) override;

		size_t receive_from(c_card_selector_base & source, unsigned char * const data, size_t size) override;
		size_t receive_from(c_cable_base_addr & source, unsigned char * const data, size_t size) override;
		void async_receive_from(unsigned char * const data, size_t size, read_handler handler) override;

		void listen_on(const c_card_selector & local_address) override;

		virtual void set_sockopt_timeout(std::chrono::microseconds timeout) override;

		virtual void stop_threadsafe() override;

		bool has_separate_rw() const noexcept; ///< do we use separate read and write socket, or is this the same socket
		static t_multisocket_kind default_multisocket_kind(); ///< get the default strategy

	private:
		init_ptr_checker m_ptr_checker;
		t_multisocket_kind m_multisocket_kind; ///< what multisocket strategy we have

		t_socket_type m_write_socket; ///< this is also used to read, if one_rw
		t_socket_type m_read_socket;

		Mutex m_enpoint_list_mutex;
		std::list<boost::asio::ip::udp::endpoint> m_endpoint_list GUARDED_BY(m_enpoint_list_mutex);

		void set_timeout_for_socket(std::chrono::microseconds timeout, t_socket_type &socket);
		#ifdef ANTINET_socket_sockopt
		using t_native_socket = boost::asio::ip::udp::socket::native_handle_type; ///< this platforms handler for native objects
		static_assert(
			std::is_same<
				boost::asio::ip::udp::socket::native_handle_type ,
				boost::asio::ip::tcp::socket::native_handle_type >
				::value
			, "Hmm can't find same native handler for e.g. UDP sockets and TCP sockets? Then what type to pass to set_sockopt_timeout here?"
		);
		#elif defined ANTINET_cancelio
		using t_native_socket = SOCKET;
		#else
		#error "ANTINET_socket_sockopt and ANTINET_cancelio not defined"
		#endif

};

#endif // cable_UDP_HPP
