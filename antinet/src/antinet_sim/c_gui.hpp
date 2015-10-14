#ifndef C_GUI_HPP
#define C_GUI_HPP

#include "libs1.hpp"
#include "c_gui.hpp"

#include "c_object.hpp"
#include "c_node.hpp"
#include "use_opengl.hpp"

/***
The state of user interface
*/
class c_gui {
public:
	//weak_ptr<c_entity> m_selected_object; 
	std::vector<unique_ptr<c_object>>::iterator m_selected_object; ///< current object (e.g. selected for editing)

	shared_ptr<c_cjddev> m_target, m_source; ///< objects selected as Source and Target in simulation
	std::vector<unique_ptr<c_object>>::iterator m_target_switch;
	std::vector<unique_ptr<c_object>>::iterator m_source_switch;
    //    std::shared_ptr <FONT> s_font_allegl (allegro_gl_convert_allegro_font(font,AGL_FONT_TYPE_TEXTURED,500.0), [](FONT *f){allegro_gl_destroy_font(f);});

	int camera_x, camera_y, camera_z; //< position of the camera. Z is probably going to be used in some 3D rendereres
	double camera_zoom; ///< camera "position": the zoom-in (1.0 is no zoom)

	c_gui();
	void move_to_home(); ///< sets the camera position to home

	int view_x(int x) const; ///< returs the x in viewport, based on camera position
	int view_y(int y) const; ///< returs the y in viewport, based on camera position
	int view_x_rev(int x) const; ///< returs the x in viewport, based on camera position (reverse)
	int view_y_rev(int y) const; ///< returs the y in viewport, based on camera position (reverse)


};

#endif // include guard

