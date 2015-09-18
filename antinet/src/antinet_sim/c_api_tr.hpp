#ifndef C_API_TR_HPP
#define C_API_TR_HPP
#include<string>
#include <functional>
#include <queue>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "c_msg.hpp"

struct msgcjd;

typedef std::string t_nym_id; ///< a simple ID that allows to identify an ID inside my program

struct t_message {
	t_nym_id m_remote_id;
	std::string m_data;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version);

};




/* 
The class for transmission
*/

class cjdmsg;

class c_api_tr {

	public:

		/**
		Send a packet of data to recipient. 
		This is message-oriented transmission, recipient will receive all data at once.
		This is synchronized transmission, the function will return once all data is sent.

		It will throw on technical error (we discovered that we are unable to send).
		
		@param guy - the guy to whom we will send.
		@param data - the string of data to send. It can hold null bytes \0 there,pit will be correctly handled.
		*/
		virtual void write_message(msgcjd p_msg)=0;

		/**
		Return all data that we received (e.g. in background).
		Wait untill there was at least some data.
		Can implement a timeout.
		*/

		virtual void read_message(std::function<void (msgcjd)> handler)=0; /*{s_message msg; handler(std::move(msg));};*/ // handler have to be thread safe!! msg is created heare and is given to another thread by handler - handler is executed heare


		c_api_tr() = default;
		virtual ~c_api_tr() = default;
};

/**

const std::string --> c_data

*/



#endif // include guard

