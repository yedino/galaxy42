#ifndef C_SIMULATION_HPP
#define C_SIMULATION_HPP

#include "libs1.hpp"
#include "c_geometry.hpp"
#include "c_world.hpp"

extern unsigned int g_max_anim_frame;

class c_simulation {
public:
	bool m_goodbye; ///< should we exit the program
	unsigned long int m_frame_number;
	unique_ptr<c_world> m_world;

	c_simulation ();

	virtual ~c_simulation ();

	void init ();

	void main_loop ();

private:
	// allegro details:
	BITMAP *m_frame; ///< allegro frame buffer
	BITMAP *&m_screen; ///< reference to the allegro's SCREEN

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
