
#pragma once

#include <boost/any.hpp>
#include "transport/base/transp_base_addr.hpp"

class c_transport_base_obj {
	protected:
		c_transport_base_obj() = default;

	public:
		virtual ~c_transport_base_obj() = default;

		virtual void send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size)=0; ///< block function
		/**
		 * @brief receive_from block function
		 * @param source address of sender
		 * @param data pointer to prealocated buffer
		 * @param size size of prealocated buffer
		 * @return number of readed bytes
		 */
		virtual size_t receive_from(c_transport_base_addr & source, unsigned char * const data, size_t size)=0;
};


