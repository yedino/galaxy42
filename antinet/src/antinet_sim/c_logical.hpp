#ifndef C_logical_hpp
#define C_lohical_hpp

#include "libs1.hpp"

class c_logical { 
	public:
		virtual void tick()=0;
};



// @TODO wos - split into files hpp+cpp

class c_logical_ppp_task : public c_logical {
	public:
		int m_state; // 0-idle 1-done  2-Someone asked us for price   3-we have greement ......
};

// c_object.hpp

class c_logical_ppp_user : public c_logical {
	public:
		void handle_command_price_request()=0;
		void handle_command_price_reply()=0;

		void handle_command_price_agreed_price()=0;
};

#endif // include guard

