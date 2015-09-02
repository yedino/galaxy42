#ifndef C_ANTINET_AGENT_API_MSG_FAILURE_H
#define C_ANTINET_AGENT_API_MSG_FAILURE_H

#include "c_antinet_agent_api_msg.h"

#include <iostream>

class c_antinet_agent_api_msg_failure : c_antinet_agent_api_msg
{
public:
    c_antinet_agent_api_msg_failure(const std::string &text);
};

#endif // C_ANTINET_AGENT_API_MSG_FAILURE_H
