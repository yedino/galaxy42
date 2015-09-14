#ifndef C_ANTINET_AGENT_API_MSG_INFO_HPP
#define C_ANTINET_AGENT_API_MSG_INFO_HPP

#include "c_antinet_agent_api_msg.hpp"

#include <iostream>

class c_antinet_agent_api_msg_info : public c_antinet_agent_api_msg {
  public:
    c_antinet_agent_api_msg_info(const std::string &text);
};

#endif // C_ANTINET_AGENT_API_MSG_INFO_HPP

