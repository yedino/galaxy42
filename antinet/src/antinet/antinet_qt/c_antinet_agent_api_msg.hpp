#ifndef C_ANTINET_AGENT_API_MSG_HPP
#define C_ANTINET_AGENT_API_MSG_HPP

#include <iostream>
#include <vector>

/// This enum numbers should be the same and backward-compatible across program versions
typedef enum {
    e_msg_kind_null = 0,
    e_msg_kind_success = 1,
    e_msg_kind_failure = 2,
    e_msg_kind_info = 3,
} t_msg_kind;

class c_antinet_agent_api_msg {
  public:
    c_antinet_agent_api_msg(const std::string &text);
    c_antinet_agent_api_msg() = default;
    virtual ~c_antinet_agent_api_msg() = default;

    t_msg_kind m_kind;
    std::string m_text;
};

#endif // C_ANTINET_AGENT_API_MSG_HPP

