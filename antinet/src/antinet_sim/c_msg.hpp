#ifndef C_MSG_HPP
#define C_MSG_HPP

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>

#include "c_object.hpp"


typedef int t_pos; ///< position (one coordinate)
typedef std::string t_cjdaddr; // cjdns address example
typedef unsigned long long int t_ID;
typedef long long int t_dht_addr;


template <class T>
class crypto_hash {
public:
	T object; // the "hashed" object
};

/**
 * @brief The e_currency enum
 * @TODO just a test, replace later
 * enum class e_currency { e_currency_USD  USD, BTC, TOKEN_A, TOKEN_B, TOKEN_C };
 */
enum class e_currency { USD, BTC, TOKEN_A, TOKEN_B, TOKEN_C };

/**
 * @struct item_netaccess
 * @brief The item_netaccess struct
 */
struct item_netaccess {
	unsigned int min_amount;
	unsigned int price;
	unsigned int quality_speed; // KB/s
	e_currency curr;
};

/**
 * \enum t_msgkind
 *	\brief kind of message
 *
 */
typedef enum : uint8_t {
    e_msgkind_default,
    e_msgkind_error, /// some error? please restart? operation not possible

    e_msgkind_buy_net_inq, /// inquery, tell me prices
    e_msgkind_buy_net_menu, /// this are my prices
    e_msgkind_buy_net_buying, /// ok I buy
    e_msgkind_buy_net_agreed, /// agreed, you bought it
    e_msgkind_buy_net_final, /// agreed, you bought it - ok see you

	e_msgkind_data,
	e_msgkind_ping_request,
	e_msgkind_ping_response,

	e_msgkind_dht_hello,
	
	e_msgkind_buy_currency
} t_msgkind;

/**
 * \struct msg
 * \brief a general message.
 *
 * a general message. e.g.: a direct message, something that is sent over a network direct link
 * sender and recipient fields are known from the cointainer that has this object
 *
 */
struct msg { // a general message. e.g.: a direct message, something that is sent over a network direct link
	// sender and recipient fields are known from the cointainer that has this object
	virtual std::string serialize() = 0;
	virtual void deserialize(std::string binary) = 0;
	virtual ~msg () = default;
};

/**
 * \class msgcjd
 *	\brief a message targetted at cjdns node
*/
struct msgcjd : public msg { // a message targetted at cjdns node
	virtual ~msgcjd () = default;
	msgcjd() = default;
public:
	unsigned int m_ttl = 100; // time to live
	t_cjdaddr m_to, m_from;
	t_msgkind m_logic;
	t_cjdaddr m_destination;
	t_ID m_ID;

	msgcjd (const t_msgkind &logic);
	virtual std::string serialize();
	virtual void deserialize(std::string binary);
private:
	friend class boost::serialization::access;
	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);
};


struct msg_dht_hello :public msgcjd {
	msg_dht_hello();
	bool m_direction;
	t_dht_addr m_target_dht_address;
	t_dht_addr m_home_dht_address;
	map<t_dht_addr,list < t_cjdaddr> > m_known_nodes;

	virtual std::string serialize() override;

	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);

};

struct msg_buy : public msgcjd {
public:
	msg_buy (const t_msgkind &logic);

	virtual ~msg_buy () = default;
};

/**
 * @struct msg_ping
 * @brief send only to neighbor
 */
struct msg_ping_request : public msgcjd {
public:
	msg_ping_request();
	virtual ~msg_ping_request() = default;

private:
	friend class boost::serialization::access;
	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);
};

/**
 * @struct msg_ping_response
 * @brief recv only from neighbor
 */
struct msg_ping_response : public msgcjd {
public:
	msg_ping_response();
	virtual std::string serialize();
	unsigned int m_ping_time;
	virtual ~msg_ping_response() = default;
private:
	friend class boost::serialization::access;
	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);
};

/**
 * @struct msg_buy_inq
 * @brief The msg_buy_inq struct
 */
struct msg_buy_inq : public msg_buy {
public:
	msg_buy_inq ();
};


/**
 * @struct msg_buy_menu
 * @brief The msg_buy_menu struct
 */
struct msg_buy_menu : public msg_buy {
public:
	//vector< item_netaccess > offer;
	msg_buy_menu ();

	unsigned int m_my_price;
private:
	friend class boost::serialization::access;
	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);
};


/**
  *@class msg_buy_buying
 * @brief The msg_buy_buying struct
 */
struct msg_buy_buying : public msg_buy {
	item_netaccess my_pick; // I picked this option
	unsigned int amount; // how much I buy 
public:
	msg_buy_buying ();
};

struct msg_buy_agreed : public msg_buy {
//	crypto_hash<msg_buy_buying> our_agreement; // we repeat our agreement
public:
	msg_buy_agreed ();
};

struct msg_buy_final : public msg_buy {
//	crypto_hash<msg_buy_buying> our_agreement; // we repeat our agreement
public:
	msg_buy_final ();
};

struct msg_buy_currency_inq : public msg_buy {
	string bid_currency;
	string ask_currency;
	unsigned int number_of_bid_currency;
public:
	msg_buy_currency_inq ();
};

struct msg_buy_currency_menu : public msg_buy {
public:
		msg_buy_currency_menu();
};

/***
 * @struct msg_use
 * @brief we somehow use the data, e.g. we use the cjdns traffic (that we paid for) to reach some service
 */
struct msg_use
	: public msgcjd { // we somehow use the data, e.g. we use the cjdns traffic (that we paid for) to reach some service
	uint8_t m_type; // type of protocol TODO
	string m_data; // example data to store
	pair<string, unsigned int> m_payment;
private:
	friend class boost::serialization::access;
	template <class Archive >
	void serialize(Archive &ar, const unsigned int version);

public:
	msg_use ();
};

struct msg_use_ftp : public msg_use {
public:
	msg_use_ftp ();
};


class c_cjddev;

/***
 @struct c_msgtx
 @brief a message in transfer in direct transfer over direct link
 */
// TODO rm
struct c_msgtx { // a message in transfer in direct transfer over direct link
	shared_ptr<msgcjd> m_msg; // <--- different kinds of messages
	weak_ptr<c_cjddev> m_otherside; // <--- PTR to the other side of connection (recipient or sender). Could be nullptr if we do not know him (yet)

	int m_animframe; // animation frame number/time
};


#endif // C_MSG_HPP
