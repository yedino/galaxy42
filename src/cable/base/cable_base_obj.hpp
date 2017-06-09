
#pragma once

#include <boost/any.hpp>
#include <functional>
#include <memory>
#include "cable/base/cable_base_addr.hpp"

#include <cable/selector.hpp>
#include "someio.hpp"

/**
 * @section unblock
 * We need a way to avoid infinite waiting for both blocking and async operations;
 * This is done by providing thread-safe method unblock_and_stop().
 * It will provide a way to unblock eventually the blocking-reads (e.g. by heaving socket option of timeout)
 * and to unblock async reads (e.g. by calling io_service.stop() for ASIO).
*/
class c_card_selector;
class c_card_selector_base;

using write_handler = const std::function<void(const unsigned char *, std::size_t)> &;
using read_handler = const std::function<void(const unsigned char *, std::size_t, c_card_selector_base &)> &;

class c_cable_base_obj : public c_someio {
	protected:
		c_cable_base_obj()=default; // to be used by factory

	public:
		using t_asio_buffers_receive = std::vector< boost::asio::mutable_buffer >;
		using t_asio_buffers_send = std::vector< boost::asio::const_buffer >;

		virtual ~c_cable_base_obj() = default;

		/**
		 * @brief send_to
		 * @param dest destination address
		 * @param data pointer to data buffer
		 * @param size size of buffer ponted by data
		 * @throws std::invalid_argument if dst is bad address type
		 * @throws std::runtime_error if send error
		 */
		virtual void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size)=0; ///< blocking function

		/**
		 * @brief send_to
		 * @param dest destination address
		 * @param buffers vector of asio buffers
		 * @throws std::invalid_argument if dst is bad address type
		 * @throws std::runtime_error if send error
		 */
		virtual void send_to(const c_cable_base_addr & dest, const t_asio_buffers_send & buffers)=0; ///< blocking function, gather buffers

		/**
		 * @brief async_send_to
		 * @param dest Destination address
		 * @param data Pointer to buffer of data to send
		 * @param size Size of buffer
		 * @param handler The handler to be called when the send operation completes. The function signature of the handler must be:\n
		 * void handler(\n
		 * 	const unsigned char *data, // send data pointer, address is equal async_send_to data parameter\n
		 * 	size_t size // Number of sended bytes\n
		 * );
		 * @throws std::invalid_argument if dest is bad address type
		 * @throws std::runtime_error if send error
		 */
		virtual void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler)=0;

		/**
		 * @brief receive_from block function
		 * @param source: source address of sender will be written here
		 * @param data pointer to prealocated buffer
		 * @param size size of prealocated buffer
		 * @return number of readed bytes
		 * @throws std::invalid_argument if source is bad address type
		 * @throws std::runtime_error if receive error
		 */
		virtual size_t receive_from(c_cable_base_addr & source, unsigned char * const data, size_t size)=0;

		virtual size_t receive_from(c_card_selector_base & source, unsigned char * const data, size_t size)=0;

		/**
		 * @brief async_receive_from
		 * @param source address of sender Ownership of the source object is retained by the caller, which must guarantee that it is valid until the handler is called.
		 * @param data pointer to prealocated buffer
		 * @param size size of prealocated buffer
		 * @param handler The handler to be called when the receive operation completes. The function signature of the handler must be:\n
		 * void handler(\n
		 * 	const unsigned char *data, // receive data pointer, address is equal async_receive_from data parameter\n
		 * 	size_t size // Number of sended bytes\n
		 * );
		 */
		virtual void async_receive_from(unsigned char * const data, size_t size, read_handler handler)=0;

		virtual void listen_on(const c_card_selector & local_address)=0;

		/// on some native socket given by caller, sets the timeout, used for blocking read - latency of exit flag #m_stop
		/// call this from child classes on all sockets (e.g. asio sockets); see #unblock
		virtual void set_sockopt_timeout(std::chrono::microseconds timeout) = 0;
};
