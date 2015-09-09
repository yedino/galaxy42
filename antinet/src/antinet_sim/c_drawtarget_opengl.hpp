#ifndef C_DRAWTARGET_HPP
#define C_DRAWTARGET_HPP

#include "libs1.hpp"
#include "c_geometry.hpp"

/***
@file OpenGL rendering support. See doc/rendering.txt
*/


/***
OpenGL layer, contains the layer number.
It must be generic/compatible enough to compile even when OpenGL library is missing or disabled.
If you need any details, e.g. some OGL specific variables here, then e.g. add a PIMPL here.
*/
class c_layer_opengl { ///< a single layer of OpenGL drawing target
public:
	const t_layer_nr m_layer_nr; ///< number of this layer

	c_layer_opengl (t_layer_nr layer_nr);

	virtual ~c_layer_opengl () = default;
};


/***
OpenGL layer, contains the layer number.
It must be generic/compatible enough to compile even when OpenGL library is missing or disabled.
If you need any details, e.g. some OGL specific variables here, then e.g. add a PIMPL here.
*/
class c_drawtarget_opengl : public c_drawtarget { ///< OpenGL library draw target (with layers, usually? e.g. in the flat 3d layers mode - see doc/rendering.txt)
public:
	c_drawtarget_opengl();

	virtual ~c_drawtarget_allegro () = default;
};


#endif // include guard

