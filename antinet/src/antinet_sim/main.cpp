#include <allegro.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "c_simulation.hpp"
#include "c_world.hpp"
#include "c_object.hpp"

using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::vector;
using std::string;

void start_the_gui_allegro() {
	int bpp = -1; // bits per pixel

	if (0 != allegro_init()) {
		std::string errmsg("Error: Can not start allegro graphics library");		std::cerr<<errmsg<<std::endl;
		throw std::runtime_error(errmsg);
	}

	install_keyboard();
	install_mouse();
	install_timer();

	bpp = 32; // colors

	set_color_depth(bpp);
	int ret = set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1280, 720, 0, 0);
	if (ret != 0) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Error starting graphical mode (bpp = %d) bit graphics mode\nError was:%s\n", bpp, allegro_error);
		std::string errmsg("Error: Can not start allegro graphics library, allegro_error=" + std::string(allegro_error));		std::cerr<<errmsg<<std::endl;
		throw std::runtime_error(errmsg);
	}
}

void start_the_gui() {
	start_the_gui_allegro();
}

void stop_the_gui() {
		clear_keybuf();
}


int main(int argc, char *argv[]) {
	try {
		start_the_gui();

		try {
			auto simulation = make_shared<c_simulation>();
			simulation->init();
			simulation->main_loop();
		}
		catch(std::runtime_error &e) { std::cerr<<std::endl<<"The main simulation caused exception: " << e.what() << std::endl; }
		catch(...) { }

		c_bitmaps::deinit();
		stop_the_gui();
	}
	catch(...) { }
	return 0;
}

END_OF_MAIN()

