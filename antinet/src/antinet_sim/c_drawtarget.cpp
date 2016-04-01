#include "c_drawtarget.hpp"


// ==================================================================

c_layer::c_layer(t_layer_nr nr)
	: m_layer_nr(nr)
{
}

// ==================================================================

string t_drawtarget_type_to_string( t_drawtarget_type x) { ///< name of the enum type as string (e.g. for debug), not for real use!!
	switch(x) {
		case e_drawtarget_type_allegro: return "Allegro";
		case e_drawtarget_type_opengl: return "OpenGL";
		// null "(null)" @TODO --eve
		default: return "(unknown!)"; // <== return
	};
}

// ==================================================================

c_drawtarget::c_drawtarget (t_drawtarget_type p_drawtarget_type)
:
	m_drawtarget_type(p_drawtarget_type),
	m_gui(nullptr),
	m_layer()
{
}

// ==================================================================



// ==================================================================
// ALLEGRO
// ==================================================================


c_layer_allegro::c_layer_allegro (t_layer_nr layer_nr, BITMAP *frame) : c_layer(layer_nr), m_frame(frame) { }

// ==================================================================


c_drawtarget_allegro::c_drawtarget_allegro (BITMAP *frame) : c_drawtarget(e_drawtarget_type_allegro) {
	// create all the layers possible:
	// TODO do this nicer - enumerate enum
	for (int nr = (int)e_layer_nr_bgr; nr < (int)e_layer_nr_top; ++nr) {
		m_layer.push_back(new c_layer_allegro( static_cast<t_layer_nr>(nr) , frame));
	}
}
c_drawtarget_allegro::~c_drawtarget_allegro() {
	for(auto lay : m_layer) {
		delete lay;
	}
}
