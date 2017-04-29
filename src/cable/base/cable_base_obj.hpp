
#pragma once

#include <boost/any.hpp>
#include <functional>
#include <memory>
#include "cable/base/cable_base_addr.hpp"

#include <cable/selector.hpp>
#include "someio.hpp"

class c_card_selector;

using write_handler = const std::function<void(const unsigned char *, std::size_t)> &;
using read_handler = const std::function<void(const unsigned char *, std::size_t, c_card_selector &)> &;

class c_cable_base_obj : public c_someio {
	protected:
		c_cable_base_obj()=default; // to be used by factory

	public:
		virtual ~c_cable_base_obj() = default;

		virtual void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size)=0; ///< block function

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
		 */
		virtual void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler)=0;

		/**
		 * @brief receive_from block function
		 * @param OUT: source address of sender will be written here
		 * @param data pointer to prealocated buffer
		 * @param size size of prealocated buffer
		 * @return number of readed bytes
		 */
		virtual size_t receive_from(c_card_selector & source, unsigned char * const data, size_t size)=0;

		/**
		 * @brief async_receive_from
		 * @param source address of sender Ownership of the source object is retained by the caller, which must guarantee that it is valid until the handler is called.
		 * @param data pointer to prealocated buffer
		 * @param size size of prealocated buffer
		 * @param handler The handler to be called when the receive operation completes. The function signature of the handler must be:\n
		 * void handler(\n
		 *	const unsigned char *data, // receive data pointer, address is equal async_receive_from data parameter\n
		 * 	size_t size // Number of sended bytes\n
		 * );
		 */
		virtual void async_receive_from(unsigned char * const data, size_t size, read_handler handler)=0;

		virtual void listen_on(const c_card_selector & local_address)=0;


};
