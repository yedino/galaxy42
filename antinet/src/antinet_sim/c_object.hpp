#ifndef C_OBJECT_HPP
#define C_OBJECT_HPP

#include "libs1.hpp"
#include "c_drawtarget.hpp"
#include "loadpng.hpp"
#include "c_bitmaps.hpp"
#include "c_api_tr.hpp"
#include "c_msg.hpp"

extern unsigned int g_max_anim_frame;

typedef int t_pos; ///< position (one coordinate)
typedef std::string t_cjdaddr; // cjdns address example
typedef unsigned long long int t_ID;
typedef long long t_dht_addr;
typedef std::string t_nym_id; ///< a simple ID that allows to identify an ID inside my program


//typedef int t_cjdaddr; // cjdns address example
//typedef int t_ID;

typedef enum {
	e_switch,
	e_node
} t_device_type;

struct c_msgtx;
struct t_message;
class c_node;

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
	virtual void logic_tick() = 0;
	virtual void recv_tick() = 0;
	virtual void send_tick() = 0;
	
	virtual void draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any);
	virtual void draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any);

	c_object (string name);

	const string& get_name() const; ///< get the name
//	virtual const t_uuid get_uuid() = 0;
	
	virtual ~c_object () = default;

    virtual t_pos get_x() = 0;
    virtual t_pos get_y() = 0;

	virtual t_device_type get_type() = 0;


    void set_font(std::shared_ptr<FONT> f) {
        s_font_allegl=f;
    }

    std::shared_ptr<FONT> s_font_allegl;
	
	bool m_selected = false;
    bool m_target = false;
    bool m_source = false;
	
	virtual void print(std::ostream & oss) const;
	friend std::ostream& operator<<(std::ostream & oss, const c_object & obj);
	
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


struct s_remote_host {
	t_cjdaddr m_address = "";
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


#endif // C_OBJECT_HPP
