#ifndef COIN_SIGNING_C_WALLET_HPP
#define COIN_SIGNING_C_WALLET_HPP
#include <string>
#include <fstream>
#include <ctime>
#include <list>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "c_token.hpp"


using std::string;
using std::list;

struct c_wallet {
    list<c_token> m_tokens;

    c_wallet();
    c_wallet(const string &);

    c_wallet(string &&) noexcept;

    size_t amount() const;

    /// verbouse == true : means that for each token all chainsign will be print
    void print_wallet_status(std::ostream &, bool verbouse = 0) const;
	void add_token (const c_token &);
    bool process_token() const;
    void remove_token (const c_token &);

    void save_to_file(const std::string &filename);
    void load_from_file(const std::string &filename);

 // private:

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & m_tokens;
    }
};


#endif //COIN_SIGNING_C_WALLET_HPP
