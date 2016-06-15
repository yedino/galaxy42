#ifndef COIN_SIGNING_C_WALLET_HPP
#define COIN_SIGNING_C_WALLET_HPP

#include "libs01.hpp"
#include "c_token.hpp"

using std::string;
using std::list;

class c_wallet {
  public:
    c_wallet ();
    c_wallet (const string &);
    c_wallet (string &&) noexcept;

    /// keep_in_wallet = 1 means double spending try
    /// should be used only in tests!
    bool process_token () const;
    c_token get_any_token (bool keep_in_wallet = 0);

    size_t amount () const;
    void move_token (c_token &&);

    size_t clean_expired_tokens ();

    /// verbouse == true : means that for each token all chainsign will be print
    void print_wallet_status (std::ostream &, bool verbouse = 0) const;

    void save_to_file (const std::string &filename) const;
    void load_from_file (const std::string &filename);

  private:
    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive &ar, const unsigned int version) {
        UNUSED(version);
        ar & m_tokens;
    }

    list<c_token> m_tokens;
    void remove_token (const c_token &);
};


#endif //COIN_SIGNING_C_WALLET_HPP
