#ifndef C_OBJECT_HPP
#define C_OBJECT_HPP

#include "libs1.hpp"
#include "c_drawtarget.hpp"
#include "loadpng.hpp"
#include "c_bitmaps.hpp"
#include "c_api_tr.hpp"
#include "c_network.hpp"
#include "c_msg.hpp"
#if defined USE_API_TR
#include "c_netdev.hpp"
#endif

extern unsigned int g_max_anim_frame;

typedef int t_pos; ///< position (one coordinate)
typedef std::string t_cjdaddr; // cjdns address example
typedef unsigned long long int t_ID;
typedef long long t_dht_addr;


//typedef int t_cjdaddr; // cjdns address example
//typedef int t_ID;

struct c_msgtx;

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

    void set_font(std::shared_ptr<FONT> f) {
        s_font_allegl=f;
    }

    std::shared_ptr<FONT> s_font_allegl;
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
		
	double get_distance(const c_entity &entity);

	
	virtual ~c_entity () = default;
};


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
 * \class c_netdev
 *
 * \brief a networked (e.g. connected somewhere) device
 *
 *
 */

#if defined USE_API_TR
#else
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
/*#if defined USE_API_TR
protected:
	// c_api_tr
	//virtual void write_message(t_message&& msg) override;
	//virtual void read_message(std::function<void (t_message &&)> handler) override;
	virtual void hw_send (t_nym_id addr,std::string &&serialized_msg) override; ///< send to remote node
	std::vector<t_message> m_raw_outbox; // general box with messages to be sent somehow
	std::vector<t_message> m_raw_inbox; // general box with messages that are received somehow
public:
	void set_network(std::shared_ptr<c_network> network_ptr);
#endif*/
};
#endif
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
#if defined USE_API_TR
class c_cjddev : public c_entity { // a cjdns-networked device. has ipv6 address from cjdns
#else
class c_cjddev : public c_netdev { // a cjdns-networked device. has ipv6 address from cjdns
#endif
protected:
#if defined USE_API_TR
	friend class c_network;
	c_netdev m_netdev;
#endif
	t_cjdaddr m_my_address;
	map<t_cjdaddr, weak_ptr<c_cjddev >> m_neighbors; ///< addr => peer ptr
	map<t_cjdaddr, unsigned int> m_neighbors_prices; ///< addr => price
	map<t_cjdaddr, s_remote_host> m_routing_table; ///< remote host => next hop (neighbor). Which peer is the correct way to go there
	unordered_set<t_cjdaddr> m_wait_hosts; ///< I'm waiting for ...
    map<t_ID, t_cjdaddr> m_response_nodes; ///< ID => addr
    
	//////////////////////////////////////////////////////////////////////////
	/////////////////dht section///////////////////////////////////

	t_dht_addr m_dht_addr;


	map<t_dht_addr,list < t_cjdaddr> > m_known_nodes;

    
    
    //

	void hw_recived(t_message);

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
