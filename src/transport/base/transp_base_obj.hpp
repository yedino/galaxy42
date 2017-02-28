
#pragma once

#include <boost/any.hpp>

class c_transport;

class c_transport_base_obj {
	public:
		c_transport_base_obj();
		virtual ~c_transport_base_obj();

		virtual void send_data(boost::any dest, const char *data, size_t size_of_data);
		// virtual void send_data_async(boost::any dest, const char *data, size_t size_of_data); // TODO
};


