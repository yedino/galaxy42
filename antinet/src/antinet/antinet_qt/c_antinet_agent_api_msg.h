#ifndef C_ANTINET_AGENT_API_MSG_H
#define C_ANTINET_AGENT_API_MSG_H

#include "c_antinet_agent_api_msg_success.h"
#include "c_antinet_agent_api_msg_failure.h"
#include "c_antinet_agent_api_msg_info.h"

#include <iostream>
#include <vector>

/// This enum numbers should be the same and backward-compatible across program versions
typedef enum {
    e_msg_kind_null = 0,
    e_msg_kind_success = 1,
    e_msg_kind_failure = 2,
    e_msg_kind_info = 3,
} t_msg_kind;




class c_antinet_agent_api_msg
{
public:
    c_antinet_agent_api_msg(const std::string &text);
    virtual ~c_antinet_agent_api_msg() = default;

    const t_msg_kind m_kind;
    const std::string m_text;
};

#endif // C_ANTINET_AGENT_API_MSG_H
