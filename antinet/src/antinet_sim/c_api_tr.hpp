#ifndef C_API_TR_HPP
#define C_API_TR_HPP


typedef std::string t_nym_id; ///< a simple ID that allows to identify an ID inside my program

struct t_message {
	t_nym_id m_remote_id;
	std::string m_data;
};


struct t_hw_message{
	t_message msg;
	enum type;
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
		virtual void write_message(t_message&& msg) = 0;

		/**
		Return all data that we received (e.g. in background).
		Wait untill there was at least some data.
		Can implement a timeout.
		*/

		virtual void read_message(std::function<void (t_message &&)> handler) = 0; /*{s_message msg; handler(std::move(msg));};*/ // handler have to be thread safe!! msg is created heare and is given to another thread by handler - handler is executed heare


		c_api_tr() = default;
		virtual ~c_api_tr() = default;
};

/**

const std::string --> c_data

*/



#endif // include guard

