#ifndef C_ANTINET_AGENT_API_MSG_FAILURE_HPP
#define C_ANTINET_AGENT_API_MSG_FAILURE_HPP

#include "c_antinet_agent_api_msg.hpp"

#include <iostream>

class c_antinet_agent_api_msg_failure : public c_antinet_agent_api_msg {
  public:
    c_antinet_agent_api_msg_failure(const std::string &text);
};

#endif // C_ANTINET_AGENT_API_MSG_FAILURE_HPP

