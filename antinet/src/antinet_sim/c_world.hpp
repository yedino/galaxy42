#ifndef C_WORLD_HPP
#define C_WORLD_HPP

#include <vector>
#include <memory>

#include "c_drawtarget.hpp"
#include "c_object.hpp"

using std::vector;
using std::shared_ptr;

class c_world {
public:
	vector<shared_ptr<c_object> > m_objects;
	shared_ptr<c_tnetdev> m_target, m_source;

	void add_test ();

	void tick ();

	void draw (c_drawtarget &drawtarget, unsigned int anim_frame);

	void connect_nodes (shared_ptr<c_object> first, shared_ptr<c_object> second);

	void load (const string &filename);

	friend ostream &operator<< (ostream &stream, const c_world &world);
};

ostream &operator<< (ostream &stream, const c_world &world);

#endif // C_WORLD_HPP
