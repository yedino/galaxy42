#ifndef C_WORLD_HPP
#define C_WORLD_HPP

#include <vector>
#include <memory>

#include "c_drawtarget.hpp"
#include "c_object.hpp"
#include "c_network.hpp"

using std::vector;
using std::shared_ptr;

typedef long double t_simclock;

/***
The logical world of the simulation, includes simulated entities.
*/
class c_world {


	/***
	 * clock that dictates flow of the simulated events
	 */
	 t_simclock m_simclock; ///< @TODO wos


public:
	vector<shared_ptr<c_object> > m_objects; ///< all the objects in simulation!
	
	// additional situation in this world:
	shared_ptr<c_tnetdev> m_target, m_source; ///< current source and target used in tests
	
	static inline t_simclock get_time();
	static const inline t_simclock get_chronon(){return 0.1;}

	std::shared_ptr <c_network> m_network;

	c_world();
	
	void add_test ();

	void tick ();

	void draw(c_drawtarget &drawtarget); ///< tells the world to draw all it's objects

	void connect_nodes (shared_ptr<c_object> first, shared_ptr<c_object> second);

	void load (const string &filename);

	friend ostream &operator<< (ostream &stream, const c_world &world);

};

ostream &operator<< (ostream &stream, const c_world &world);

#endif // C_WORLD_HPP
