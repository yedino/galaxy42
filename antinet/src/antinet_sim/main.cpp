/***
@file This is the file with entry point main() of the application in this sub project.
*/

#include <allegro.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <set>

#include "c_simulation.hpp"
#include "c_world.hpp"
#include "c_object.hpp"

#include "use_opengl.hpp"

using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::vector;
using std::string;



void start_gui_part_allegro() {
	_note("Loading the Allegro library (general)");
	
	if (0 != allegro_init()) {
		std::string errmsg("Error: Can not start allegro graphics library");		std::cerr<<errmsg<<std::endl;
		throw std::runtime_error(errmsg);
	}
}

void start_gui_allegro_simple() {
	_note("Starting GUI - for simple Allegro");
	
	start_gui_part_allegro();
	
	int bpp = -1; // bits per pixel

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


void start_gui_allegro_opengl() {
    //int bpp = -1; // bits per pixel
	_note("Starting GUI - for OpenGL and AllegroGL");
	
	start_gui_part_allegro();

	install_allegro_gl();
	install_keyboard();
	install_mouse();
	install_timer();
	
	allegro_gl_clear_settings();
	allegro_gl_set(AGL_COLOR_DEPTH, 32);
	allegro_gl_set(AGL_Z_DEPTH, 24);
	allegro_gl_set(AGL_WINDOWED, TRUE);
	allegro_gl_set(AGL_DOUBLEBUFFER, 1);
	allegro_gl_set(AGL_RENDERMETHOD, 1);
	allegro_gl_set(AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_DOUBLEBUFFER
								 | AGL_RENDERMETHOD | AGL_Z_DEPTH | AGL_WINDOWED);
	
	if (set_gfx_mode(GFX_OPENGL, 1280, 720, 0, 0)) {
		allegro_message("Error setting OpenGL graphics mode:\n%s\n"
										"Allegro GL error : %s\n",
										allegro_error, allegro_gl_error);
		//return -1;
	}
	
	//allegro_gl_convert_allegro_font(font, AGL_FONT_TYPE_TEXTURED, 16.0);
	
	
	// --- scene ---
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
	glViewport(0, 0, SCREEN_W, SCREEN_H);
	
	// the PROJTECTION matrix
	// do not change this, e.g. do NOT move "camera" here https://www.opengl.org/archives/resources/faq/technical/projection_abuse.php
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// glFrustum(-1.0, -1.0, -1.0, -1.0, 1, 60.0);
    glFrustum(0.0, 10.0, -1.0, 1.0, 1, 60.0); // good position
	
	
	// glEnable(GL_CULL_FACE);
	// glFrontFace(GL_CCW);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
    glClearColor(255,255,255,0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear it once
	allegro_gl_flip(); // show the sreen
	
	if (1) { // TODO why???
		allegro_gl_set_allegro_mode(); // <=== in allegro simple mode ! ??? why
		// allegro_gl_unset_allegro_mode(); // <=== in opengl mode !
	}
}


void start_the_gui(t_drawtarget_type drawtarget_type) {
	_note("Starting a GUI");
	if (drawtarget_type == e_drawtarget_type_allegro) {
		start_gui_allegro_simple();
	}
	if (drawtarget_type == e_drawtarget_type_opengl) {
		start_gui_allegro_opengl();
	}
}

void stop_the_gui() {
		clear_keybuf();
}


int main(int argc, char *argv[]) {
	std::cerr << "============================================" << std::endl;
	std::cerr << "The simulation program - part of AntiNet.org project" << std::endl;
	std::cerr << "============================================" << std::endl;
	std::cerr << "WARNING: This is a very early pre-alpha, do not use this!" << std::endl;
	std::cerr << "Do not even run this at any real user, it likely contains errors, UBs, or exploits!" << std::endl;
	std::cerr << "Test on separate user/environment until we have a tested version." << std::endl;
	std::cerr << "============================================" << "\n" << std::endl;

	try {
	
		std::set< std::string > args;
		for (int i=1; i<argc; ++i) args.insert( argv[i] );  // get arguments into args

		t_drawtarget_type drawtarget_type = e_drawtarget_type_allegro;
		int drawtarget_type_count=0; // set to true if user specified too many (e.g. more then 1) draw target. E.g. to warn him
		if (args.count("--gl")==1 || args.count("--opengl")) { drawtarget_type = e_drawtarget_type_opengl;  ++drawtarget_type_count; }
		if (args.count("--alleg")==1 || args.count("--allegro")) { drawtarget_type = e_drawtarget_type_allegro;  ++drawtarget_type_count; }
		if (drawtarget_type_count>1) std::cerr<<"Warning: you specified few conflicting drawing engines in the program command line options. (will ignore some)"<<std::endl;

		start_the_gui( drawtarget_type );

		try {
			auto simulation = make_shared<c_simulation>(  drawtarget_type  );
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

