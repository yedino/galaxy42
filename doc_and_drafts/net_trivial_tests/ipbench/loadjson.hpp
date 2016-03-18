
#include <jsoncpp/json/json.h>

struct c_peer {
    uint32_t m_nr;		///< peer number (could be usefull in routing)
    std::string m_ip;
    std::string m_public_key;
};

class c_json_file_parser {
  public:
    c_json_file_parser (const std::string &filename);

    c_json_file_parser () = delete;
    c_json_file_parser (const c_json_file_parser &) = delete;

    std::vector<c_peer> get_peers();

  private:
    bool parse_file(const std::string &filename);
    Json::Value root;   // will contains the root value after parsing.
};
