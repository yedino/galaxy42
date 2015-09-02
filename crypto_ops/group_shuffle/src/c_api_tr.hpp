#ifndef C_API_TR_HPP
#define C_API_TR_HPP

#include "libs1.hpp"

#include "c_api_wot.hpp"


struct s_message {
	t_nym_id m_source_id;
	std::string m_data;
};

/* 
The class for transmission
*/

class c_api_tr {

	public:

		/**
		Send a packet of data to recipient. 
		This is message-oriented transmission, recipient will receive all data at once.
		This is synchronized transmission, the function will return once all data is sent.

		It will throw on technical error (we discovered that we are unable to send).
		
		@param guy - the guy to whom we will send.
		@param data - the string of data to send. It can hold null bytes \0 there, it will be correctly handled.
		*/
		virtual void write_to_nym(t_nym_id guy, const std::string & data)=0;

		/**
		Return all data that we received (e.g. in background).
		Wait untill there was at least some data.
		Can implement a timeout.
		*/
		virtual vector<s_message> read_or_wait_for_data()=0;

		c_api_tr() = default;
		virtual ~c_api_tr() = default;
};

/**

const std::string --> c_data

*/



#endif // include guard

