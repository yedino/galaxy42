
#pragma once

#include <boost/any.hpp>
#include "transport/base/transp_base_addr.hpp"

class c_transport_base_obj {
	protected:
		c_transport_base_obj();

	public:
		virtual ~c_transport_base_obj() = default;

		virtual void send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size)=0;
};


