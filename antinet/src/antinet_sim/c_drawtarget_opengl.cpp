#include "c_drawtarget_opengl.hpp"



// ==================================================================
// OPENGL
// ==================================================================

c_layer_opengl::c_layer_opengl (t_layer_nr layer_nr) : c_layer(layer_nr) { }

// ==================================================================

c_drawtarget_opengl::c_drawtarget_opengl() : c_drawtarget(e_drawtarget_type_opengl) {
    // create all the layers possible:
    // TODO do this nicer - enumerate enum
    for (int nr = (int)e_layer_nr_bgr; nr < (int)e_layer_nr_top; ++nr) {
        m_layer.push_back(new c_layer_opengl(static_cast<t_layer_nr>(nr)));
    }
}
