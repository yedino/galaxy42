#ifndef C_DRAWTARGET_HPP
#define C_DRAWTARGET_HPP

#include "libs1.hpp"
#include "c_geometry.hpp"

/***
number the possible layers to be drawn e.g. for 2D view. 0 is the bottom/lowest. Should end with e_layer_top
WARNING numbers must be continous, and must start from 0.
And must begin with e_layer_nr_bgr and end with e_layer_nr_top
***/
typedef enum {
	e_layer_nr_bgr = 0, // the background
	e_layer_nr_gui_bgr, // GUI parts that are "under" all objects

	e_layer_nr_object, ///< objects e.g. routers 
	e_layer_nr_object_extra, ///< and their labels etc

	e_layer_nr_route, ///< the links
	e_layer_nr_route_extra, ///< and their labels etc

	e_layer_nr_object_activity, ///< related sub-objects, e.g. packets in them
	e_layer_nr_route_activity, ///< related sub-objects, e.g. packets in them

	e_layer_nr_gui, ///< the main gui cursor etc

	e_layer_nr_top, ///< the last layer_nr always

} t_layer_nr;

typedef enum { ///< enum 
	e_drawtarget_type_null, ///< not drawing
	e_drawtarget_type_allegro, ///< the allegro layered drawing
} t_drawtarget_type;

class c_gui;

class c_drawtarget { ///< something we can draw on to
public:
	const t_drawtarget_type m_drawtarget_type; ///< what type of drawing surface is stored in me (hint for dynamic cast)

	shared_ptr<c_gui> m_gui; // a GUI with various information

	c_drawtarget (t_drawtarget_type drawtarget_type);

	virtual ~c_drawtarget () = default;
};

class c_allegro_layer { ///< a single layer of allegro drawing target
public:
	BITMAP *const m_frame; ///< pointer-only, owned by someone else. usually a pointer to the frame
	const t_layer_nr m_layer_nr; ///< number of this layer

	c_allegro_layer (BITMAP *frame, t_layer_nr layer_nr);

	virtual ~c_allegro_layer () = default;
};


class c_drawtarget_allegro : public c_drawtarget { ///< allegro library draw target (with layers)
public:
	vector<c_allegro_layer> m_layer; ///< layers. the index is the layer number: e.g.: .at(e_layer_top).m_layer == e_layer_top
	c_drawtarget_allegro (BITMAP *frame);

	virtual ~c_drawtarget_allegro () = default;
};


#endif // include guard

