#ifndef INCLUDE_antinet_agent_lib
#define INCLUDE_antinet_agent_lib


typedef long int t_tok_amount;
typedef std::string t_tok_id;
typedef std::string t_nym_id;


/// This enum numbers should be the same and backward-compatible across program versions
typedef enum {
	e_msg_kind_success = 1,
	e_msg_kind_failure = 2,
	e_msg_kind_info = 3,
} t_msg_kind;

class c_antinet_agent_api_msg {
	public:
		std::string m_text;
		t_msg_kind m_kind;

		// TODO constr

};


class c_antinet_agent_api {

	public:
		
		/**
		* call this when we are about to exit. this can process all pending events (but does not have to), and will call a tick() itself if needed
		*/
		void hande_exiting_now(); 

		void tokens_emit(const t_tok_id &token, const t_tok_amount &amount);
		void tokens_xfer(const t_tok_id &token, const t_tok_amount &amount, const t_nym_id recipient, const t_tok_id &for_token, const t_tok_amount &for_amount);
		std::vector< c_antinet_agent_api_msg > eat_messages();

		std::vector< c_antinet_agent_api_msg > m_messsages;
};


std::vector< c_antinet_agent_api_msg > c_antinet_agent_api_msg::eat_messages() {
	std::vector< c_antinet_agent_api_msg > cpy;
	cpy.swap(m_messsages()); // also erases here
	return cpy;
}

/*
std::vector< c_antinet_agent_api_msg > c_antinet_agent_api_msg::eat_messages() {
	auto cpy; // *
	cpy.swap(m_mes);
	return cpy;
}



std::vector< c_antinet_agent_api_msg > c_antinet_agent_api_msg::eat_messages() {
//	auto cpy; // *
//	cpy.swap(m_mes);
	return std::move( m_messsages );
}
*/

#endif


