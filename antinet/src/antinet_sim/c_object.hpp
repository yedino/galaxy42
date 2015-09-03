#ifndef C_OBJECT_HPP
#define C_OBJECT_HPP

#include "libs1.hpp"
#include "c_drawtarget.hpp"
#include "loadpng.hpp"
#include "c_bitmaps.hpp"

extern unsigned int g_max_anim_frame;

typedef int t_pos; ///< position (one coordinate)
typedef unsigned long int t_cjdaddr; // cjdns address example
typedef unsigned long long int t_ID;

class c_object { // something in the simulation
public:
	virtual void tick (); ///< execute a tick of the animation
	virtual void draw (const c_drawtarget &drawtarget,
		c_layer_allegro &layer,
		int color) const; ///< draw the object (draw one layer of it, for allegro)
	virtual void draw (const c_drawtarget &drawtarget, c_layer_allegro &layer, int color, unsigned int anim_frame);

	c_object (string name);

	string get_name () const;

	virtual ~c_object () = default;

protected:
	string m_name;
	unsigned int m_animframe;
};

class c_entity : public c_object { // object in the world, in the diagram
protected:
	virtual void draw (const c_drawtarget &drawtarget,
		c_layer_allegro &layer,
		int color) const; ///< draw the object (draw one layer of it, for allegro)

public:
	t_pos m_x, m_y; ///< position in the world
	c_entity (string name, t_pos x, t_pos y);

	virtual ~c_entity () = default;
};


struct msg { // a general message. e.g.: a direct message, something that is sent over a network direct link
	// sender and recipient fields are known from the cointainer that has this object
	virtual ~msg () = default;
};

typedef enum {
	e_msgkind_default, e_msgkind_error, // some error? please restart? operation not possible

	e_msgkind_buy_net_inq, // inquery, tell me prices
	e_msgkind_buy_net_menu, // this are my prices
	e_msgkind_buy_net_buying, // ok I buy
	e_msgkind_buy_net_agreed, // agreed, you bought it
	e_msgkind_buy_net_final, // agreed, you bought it - ok see you

	e_msgkind_data,
	e_msgkind_ping,
	
	e_msgkind_buy_currency
} t_msgkind;

class c_wallet {
public:
	map<std::string, unsigned int> m_currency; // cuttency => number of currency
	void draw (BITMAP *frame, int color, t_pos x, t_pos y) const;
};

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

enum class e_currency { USD, BTC, TOKEN_A, TOKEN_B, TOKEN_C };

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

struct msg_buy_inq : public msg_buy {
public:
	msg_buy_inq ();
};

struct msg_buy_menu : public msg_buy {
public:
	//vector< item_netaccess > offer;
	msg_buy_menu ();

	unsigned int m_my_price;
};

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

struct c_msgtx { // a message in transfer in direct transfer over direct link
	shared_ptr<msgcjd> m_msg; // <--- different kinds of messages
	weak_ptr<c_cjddev> m_otherside; // <--- PTR to the other side of connection (recipient or sender). Could be nullptr if we do not know him (yet)

	int m_animframe; // animation frame number/time
};


class c_netdev : public c_entity { // a networked (e.g. connected somewhere) device
protected:
	vector<unique_ptr<c_msgtx> > m_outbox; // general box with messages to be sent somehow
	vector<unique_ptr<c_msgtx> > m_inbox;  // general box with messages that are received somehow
	vector<unique_ptr<c_msgtx> > m_oldbox; // for unreplied messages from inbox

public:
	c_netdev (string name, t_pos x, t_pos y);

	virtual ~c_netdev () = default;

	virtual void receive_message (unique_ptr<c_msgtx> &&message); // TODO ttl
	virtual unique_ptr<c_msgtx> send_message (); // TODO ttl
};

struct s_remote_host {
	t_cjdaddr m_address = 0;
	unsigned int m_price = 0;
};

class c_cjddev : public c_netdev { // a cjdns-networked device. has ipv6 address from cjdns
protected:
	t_cjdaddr m_my_address;
	map<t_cjdaddr, weak_ptr<c_cjddev >> m_neighbors; // addr => peer ptr
	map<t_cjdaddr, unsigned int> m_neighbors_prices; // addr => price
	map<t_cjdaddr, s_remote_host> m_routing_table; // remote host => next hop (neighbor)
	unordered_set<t_cjdaddr> m_wait_hosts; // I'm waiting for ...
	map<t_ID, t_cjdaddr> m_response_nodes; // ID => addr

public:
	c_cjddev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6);

	virtual ~c_cjddev () = default;

	virtual void draw (const c_drawtarget &drawtarget,
		c_layer_allegro &layer,
		int color) const; ///< draw the object (draw one layer of it, for allegro)

	void add_neighbor (shared_ptr<c_cjddev> neighbor);

	void add_neighbor (shared_ptr<c_cjddev> neighbor, unsigned int price);

	void remove_neighbor (shared_ptr<c_cjddev> neighbor);

	void remove_neighbor (t_cjdaddr address);

	virtual void receive_message (unique_ptr<c_msgtx> &&message);

	t_cjdaddr get_address () const;

	vector<t_cjdaddr> get_neighbors_addresses () const;

	vector<shared_ptr<c_cjddev >> get_neighbors () const;

	unsigned int get_price (t_cjdaddr address) const;

	void buy_net (const t_cjdaddr &destination_addr);

	virtual bool send_ftp_packet (const t_cjdaddr &destination_addr, const std::string &data);

	virtual void tick ();
};

class c_tnetdev : public c_cjddev {
public:
	c_tnetdev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6);

	virtual void draw (const c_drawtarget &drawtarget,
		c_layer_allegro &layer,
		int color) const; ///< draw the object (draw one layer of it, for allegro)
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
