#ifndef C_OBJECT_HPP
#define C_OBJECT_HPP

#include "libs1.hpp"
#include "c_drawtarget.hpp"
#include "loadpng.hpp"
#include "c_bitmaps.hpp"
#include "c_api_tr.hpp"

extern unsigned int g_max_anim_frame;

typedef int t_pos; ///< position (one coordinate)
typedef unsigned long int t_cjdaddr; // cjdns address example
typedef unsigned long long int t_ID;


/**
 *\class c_object
 *
 * \brief class to prepare simulation
 * every next step is done in next tick();
 *
 */
class c_object { // something in the simulation
public:
	virtual void tick (); ///< execute a tick of the animation
	
	virtual void draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any);
	virtual void draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any);

	c_object (string name);

	string get_name () const;

	virtual ~c_object () = default;

    virtual t_pos get_x() = 0;
    virtual t_pos get_y() = 0;
protected:
	string m_name;
	unsigned int m_animframe;
};

/**
 * \class c_entity
 * \brief object in the world, in the diagram
 *
 */
class c_entity : public c_object { // object in the world, in the diagram
protected:
	virtual void draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any);
	virtual void draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any);

public:
	t_pos m_x, m_y; ///< position in the world
    c_entity (string name, t_pos x, t_pos y);

        t_pos get_x();
        t_pos get_y();

	virtual ~c_entity () = default;
};


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
	virtual ~msg () = default;
};

/**
 * \enum t_msgkind
 *	\brief kind of message
 *
 */
typedef enum {
    e_msgkind_default,
    e_msgkind_error, /// some error? please restart? operation not possible

    e_msgkind_buy_net_inq, /// inquery, tell me prices
    e_msgkind_buy_net_menu, /// this are my prices
    e_msgkind_buy_net_buying, /// ok I buy
    e_msgkind_buy_net_agreed, /// agreed, you bought it
    e_msgkind_buy_net_final, /// agreed, you bought it - ok see you

	e_msgkind_data,
	e_msgkind_ping,
	
	e_msgkind_buy_currency
} t_msgkind;

/**
 *\class c_wallet
 *\brief wallet to pay for connection
 *
 *
*/
class c_wallet {
public:
	map<std::string, unsigned int> m_currency; ///< currency => number of currency
	void draw (BITMAP *frame, int color, t_pos x, t_pos y) const;
};

/**
 * \class msgcjd
 *	\brief a message targetted at cjdns node
*/
struct msgcjd : public msg { // a message targetted at cjdns node
	virtual ~msgcjd () = default;

public:
	unsigned int m_ttl = 100; // time to live
	t_cjdaddr m_to, m_from;
	const t_msgkind m_logic;
	t_cjdaddr m_destination;
	t_ID m_ID;

	msgcjd (const t_msgkind &logic);
};


struct msg_buy : public msgcjd {
public:
	msg_buy (const t_msgkind &logic);

	virtual ~msg_buy () = default;
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

template <class T>
class crypto_hash {
public:
	T object; // the "hashed" object
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
	crypto_hash<msg_buy_buying> our_agreement; // we repeat our agreement
public:
	msg_buy_agreed ();
};

struct msg_buy_final : public msg_buy {
	crypto_hash<msg_buy_buying> our_agreement; // we repeat our agreement
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
struct c_msgtx { // a message in transfer in direct transfer over direct link
	shared_ptr<msgcjd> m_msg; // <--- different kinds of messages
	weak_ptr<c_cjddev> m_otherside; // <--- PTR to the other side of connection (recipient or sender). Could be nullptr if we do not know him (yet)

	int m_animframe; // animation frame number/time
};


/**
 * \class c_netdev
 *
 * \brief a networked (e.g. connected somewhere) device
 *
 *
 */
class c_netdev : public c_entity, public c_api_tr{ // a networked (e.g. connected somewhere) device
protected:
	vector<unique_ptr<c_msgtx> > m_outbox; // general box with messages to be sent somehow
	vector<unique_ptr<c_msgtx> > m_inbox;  // general box with messages that are received somehow
	vector<unique_ptr<c_msgtx> > m_oldbox; // for unreplied messages from inbox

public:
	c_netdev (string name, t_pos x, t_pos y);

	virtual ~c_netdev () = default;

	virtual void receive_message (unique_ptr<c_msgtx> &&message); // TODO ttl
	virtual unique_ptr<c_msgtx> send_message (); // TODO ttl
	
protected:
	// c_api_tr
	/*
	virtual void write_to_nym(t_nym_id guy, std::string && data) override;
	virtual vector<s_message> read_or_wait_for_data() override;
	std::vector<s_message> m_raw_outbox;
	std::vector<s_message> m_raw_inbox;
	*/
};

struct s_remote_host {
	t_cjdaddr m_address = 0;
	unsigned int m_price = 0;
};


/**
 * \class c_routing_package
 * \brief package to store information about trace
 *
 * package of storing information how to trace to the target node
 * and about trace how get back;
 *
 */
struct c_routing_package
{
    pair<t_ID, t_cjdaddr> home_addr;
    pair<t_ID, t_cjdaddr> target_addr;
    vector<t_cjdaddr> home_to_target_trace;
    vector<t_cjdaddr> target_to_home_trace;

    bool direction;	//if false dircetion is home to target
};

/**
 * \class c_cjddev
 *
 * \brief a cjdns-networked device. has ipv6 address from cjdns
*/
class c_cjddev : public c_netdev { // a cjdns-networked device. has ipv6 address from cjdns
protected:
	t_cjdaddr m_my_address;
	map<t_cjdaddr, weak_ptr<c_cjddev >> m_neighbors; ///< addr => peer ptr
	map<t_cjdaddr, unsigned int> m_neighbors_prices; ///< addr => price
	map<t_cjdaddr, s_remote_host> m_routing_table; ///< remote host => next hop (neighbor). Which peer is the correct way to go there
	unordered_set<t_cjdaddr> m_wait_hosts; ///< I'm waiting for ...
    map<t_ID, t_cjdaddr> m_response_nodes; ///< ID => addr

    
    
    //m.zychowski finding dht tracing
    void add_known_trace(t_ID,vector<t_cjdaddr>);
    map<t_ID,vector<t_cjdaddr> > known_traces;
    t_ID find_nearest_to_target(t_ID target);
    void send_hello_package(t_ID target,c_routing_package package);
    void send_package(t_ID target);
    void send_by_route(t_ID target,vector<t_cjdaddr> trace_to_nearest_node);
    
    
    //


public:
	c_cjddev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6);

	virtual ~c_cjddev () = default;

	virtual void draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any);
	virtual void draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any);

	void add_neighbor (shared_ptr<c_cjddev> neighbor);

	void add_neighbor (shared_ptr<c_cjddev> neighbor, unsigned int price);

	void remove_neighbor (shared_ptr<c_cjddev> neighbor);

	void remove_neighbor (t_cjdaddr address);

	virtual void receive_message (unique_ptr<c_msgtx> &&message);

	t_cjdaddr get_address () const;

	vector<t_cjdaddr> get_neighbors_addresses () const;

	vector<shared_ptr<c_cjddev >> get_neighbors () const;

    unsigned int get_price (t_cjdaddr address) const;

//    unsigned int get_my_price() const;

	void buy_net (const t_cjdaddr &destination_addr);

    virtual bool send_ftp_packet (const t_cjdaddr &destination_addr, const std::string &data);

    int num_of_wating();

/**
 *THIS IS JUST A SIMPLE TEST!!! with very expensive full search.
 *
 * this should be in a loop to send faster - many packets at once (once full algorithm is implemented)
 *
 *	process outbox
 *	 there is something to send in the network from us
    * so send it.
 *
 *
 *
*/
	virtual void tick ();
};

/**
 * \class c_tnetdev
 *
 * \brief class with wallet
 *
 * klasa przedstawiajaca platne polaczenie
 *
 *
 */
class c_tnetdev : public c_cjddev {
public:
	c_tnetdev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6);
	
	virtual void draw_allegro(c_drawtarget &drawtarget, c_layer &layer);
	virtual void draw_opengl(c_drawtarget &drawtarget, c_layer &layer);
	

	virtual ~c_tnetdev () = default;

	virtual bool send_ftp_packet (const t_cjdaddr &destination_addr, const std::string &data);

	virtual void tick ();

protected:
	c_wallet m_wallet;
};

class c_userdev : public c_tnetdev {
public:
	c_userdev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6);
};

#endif // C_OBJECT_HPP
