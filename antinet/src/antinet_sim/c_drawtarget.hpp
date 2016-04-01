#ifndef C_DRAWTARGET_HPP
#define C_DRAWTARGET_HPP

#include "libs1.hpp"
#include "c_geometry.hpp"

/***
@file Allegro (simple 2d) rendering support. See doc/rendering.txt
*/


/***
number the possible layers to be drawn e.g. for 2D view. 0 is the bottom/lowest. Should end with e_layer_top
WARNING numbers must be continous, and must start from 0.
And must begin with e_layer_nr_bgr and end with e_layer_nr_top
***/
typedef enum {
	e_layer_nr_bgr = 0, // the background
	e_layer_nr_gui_bgr, // GUI parts that are "under" all objects

	e_layer_nr_route, ///< the links
	e_layer_nr_route_extra, ///< and their labels etc

	e_layer_nr_object, ///< objects e.g. routers
	e_layer_nr_object_extra, ///< and their labels etc

	e_layer_nr_object_activity, ///< related sub-objects, e.g. packets in them
	e_layer_nr_route_activity, ///< related sub-objects, e.g. packets in them

	e_layer_nr_gui, ///< the main gui cursor etc

	e_layer_nr_top, ///< the last layer_nr always

} t_layer_nr;

typedef enum { ///< enum 
	e_drawtarget_type_null, ///< not drawing
	e_drawtarget_type_allegro, ///< the allegro layered drawing
	e_drawtarget_type_opengl, ///< use OpenGL
} t_drawtarget_type;

string t_drawtarget_type_to_string( t_drawtarget_type x); ///< name of the enum type as string (e.g. for debug), not for real use!!


class c_gui;


/***
General class of layer of something to draw on.
Usually it will be dynamic_cast and then used.
*/
class c_layer { 
	public:
		const t_layer_nr m_layer_nr; ///< number of this layer
		
		c_layer(t_layer_nr nr);
		virtual ~c_layer()=default;
};

class c_drawtarget { ///< something we can draw on to
public:
	const t_drawtarget_type m_drawtarget_type; ///< what type of drawing surface is stored in me (hint for dynamic cast)

	shared_ptr<c_gui> m_gui; ///< a GUI with various information

	/***
	Layers. the index is the layer number: e.g.: .at(e_layer_top).m_layer == e_layer_top
	This can depend on the drawing engine.
	This will probably contain some inherited class like c_drawtarget_allegro.
	Owner: The container holds new-allocated objects that are OWNED by this object, this object must clean them up on destruction.
	Speed: we can assume there are no NULL here (after construction and init, during normal life time)
	*/
	vector<c_layer*> m_layer;

	c_drawtarget (t_drawtarget_type drawtarget_type);

	virtual ~c_drawtarget () = default;
};

// ======================================================================================================

// TODO: move this later into _allegro drawtarget and layer, like the opengl engine is separated out

class c_layer_allegro : public c_layer { ///< a single layer of allegro drawing target
public:
	BITMAP *const m_frame; ///< pointer-only, owned by someone else. usually a pointer to the frame

	c_layer_allegro (t_layer_nr layer_nr, BITMAP *frame);

	// TODO check this in doxygen
	/** default copy is safe because m_frame is only observer pointer */
	c_layer_allegro(const c_layer_allegro &) = default;
	c_layer_allegro& operator=(const c_layer_allegro &) = default;
};


class c_drawtarget_allegro : public c_drawtarget { ///< allegro library draw target (with layers)
public:
	c_drawtarget_allegro (BITMAP *frame);
	~c_drawtarget_allegro();
};


#endif // include guard

