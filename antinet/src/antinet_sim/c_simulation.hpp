#ifndef C_SIMULATION_HPP
#define C_SIMULATION_HPP

#include "libs1.hpp"
#include "c_geometry.hpp"
#include "c_world.hpp"

extern unsigned int g_max_anim_frame;

class c_simulation {
public:
	bool m_goodbye; ///< should we exit the program
	unsigned long int m_frame_number; ///< current frame of animation
	unique_ptr<c_world> m_world; ///< all the objects in this world

	c_simulation(t_drawtarget_type drawtarget_type); ///< create the simulation. choose the drawtarget type (drawing engine e.g. Allegro or OpenGL)
//c_simulation(); // t_drawtarget_type drawtarget_type); ///< create the simulation. choose the drawtarget type (drawing engine e.g. Allegro or OpenGL)

	virtual ~c_simulation ();

	void init ();

	void main_loop ();

private:

	t_drawtarget_type m_drawtarget_type; ///< the selected drawing target, implies the drawing mode. E.g. Allegro or OpenGL. Do not just change it after creation.

	// TODO disable if disabled allegro support
	// allegro details:
	BITMAP *m_frame; ///< allegro frame buffer
	BITMAP *m_screen; ///< reference to the allegro's SCREEN
	BITMAP *smallWindow; ///< allegro window for display text info

	shared_ptr<c_drawtarget> m_drawtarget; ///< drawing will be send to this
	shared_ptr<c_gui> m_gui; ///< the current GUI

	volatile static bool close_button_pressed;

	// = false;
	void static c_close_button_handler () {
		close_button_pressed = true;
	}

	shared_ptr<c_entity> get_move_object (int mouse_x, int mouse_y);
};

#endif // C_SIMULATION_HPP
