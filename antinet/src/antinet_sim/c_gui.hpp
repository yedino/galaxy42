#ifndef C_GUI_HPP
#define C_GUI_HPP

#include "libs1.hpp"
#include "c_gui.hpp"

#include "c_object.hpp"
#include "c_node.hpp"
#include "use_opengl.hpp"

#include "allegro.h" // to know format of "key"

struct t_point3gui {
	int x,y,z;
};
std::ostream & operator<<(std::ostream &ostr, t_point3gui obj);


/***
The state of user interface
*/
class c_gui {
	public:
		//weak_ptr<c_entity> m_selected_object;
		std::vector<unique_ptr<c_object>>::iterator m_selected_object; ///< current object (e.g. selected for editing)

		bool m_target_ok;		///< easy way to check, if target iterator is correctly initialized
		bool m_source_ok;		///< easy way to check, if source iterator is correctly initialized
		std::vector<unique_ptr<c_object>>::iterator m_target_node;
		std::vector<unique_ptr<c_object>>::iterator m_source_node;
			//    std::shared_ptr <FONT> s_font_allegl (allegro_gl_convert_allegro_font(font,AGL_FONT_TYPE_TEXTURED,500.0), [](FONT *f){allegro_gl_destroy_font(f);});

		int camera_x, camera_y, camera_z; //< position of the camera. Z is probably going to be used in some 3D rendereres
		double camera_zoom; ///< camera "position": the zoom-in (1.0 is no zoom)

		t_point3gui m_cursor; ///< position of the cursor "e.g. on-screen, in pixels". Better use view_x() etc.
		int m_mouseb; ///< mouse button state, in allegro format

		typedef char t_allegro_key[KEY_MAX]; ///< format of allegro keyboards keys state
		t_allegro_key m_key; ///< keys (keyboard) in allegro format

		typedef int t_allegro_shifts; ///< format of allegro keyboards shift-like-keys state
		t_allegro_shifts m_shift;

	public:

		c_gui();
		void move_to_home(); ///< sets the camera position to home

		int view_x(int x) const; ///< returs the x in viewport, based on camera position
		int view_y(int y) const; ///< returs the y in viewport, based on camera position
		int view_x_rev(int x) const; ///< returs the x in viewport, based on camera position (reverse)
		int view_y_rev(int y) const; ///< returs the y in viewport, based on camera position (reverse)

		int view_x() const; ///< returs the x in viewport, based on camera position
		int view_y() const; ///< returs the y in viewport, based on camera position
		int view_x_rev() const; ///< returs the x in viewport, based on camera position (reverse)
		int view_y_rev() const; ///< returs the y in viewport, based on camera position (reverse)
};


#endif // include guard

