#include <allegro.h>
#include <alleggl.h>
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




// Caller MUST call gl flip or similar! and must clear screen before if needed
void drawGLScene( )
{
    /* These are to calculate our fps */
//    static GLint T0     = 0;
  //  static GLint Frames = 0;

    /* Clear The Screen And The Depth Buffer */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Move Left 1.5 Units And Into The Screen 6.0 */
    glLoadIdentity();
    float rnd = (rand()%1000)/1000. ;

    glTranslatef( -0.1f * rnd, 0.0f, 0.0f );
    glBegin( GL_TRIANGLES );             /* Drawing Using Triangles       */
      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle               */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f( -1.0f, -1.0f,  0.0f ); /* Left Of Triangle              */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f(  1.0f, -1.0f,  0.0f ); /* Right Of Triangle             */
    glEnd( );                            /* Finished Drawing The Triangle */

    /* Move Right 3 Units */
    glTranslatef( 3.0f, 0.0f, 0.0f );

    /* Set The Color To Blue One Time Only */
    glColor3f( 0.5f, 0.5f, 1.0f);
/*
    glBegin( GL_QUADS );
      glVertex3f(  1.0f,  1.0f,  0.0f );
      glVertex3f( -1.0f,  1.0f,  0.0f );
      glVertex3f( -1.0f, -1.0f,  0.0f );
      glVertex3f(  1.0f, -1.0f,  0.0f );
    glEnd( );
*/
  //  SDL_GL_SwapBuffers( );

    /*
    Frames++;
    {
    GLint t = SDL_GetTicks();
    if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
        GLfloat fps = Frames / seconds;
        printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
        T0 = t;
        Frames = 0;
    }
    }
*/

}




void start_the_gui_allegro() {
    //int bpp = -1; // bits per pixel

	if (0 != allegro_init()) {
		std::string errmsg("Error: Can not start allegro graphics library");		std::cerr<<errmsg<<std::endl;
		throw std::runtime_error(errmsg);
	}
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

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glViewport(0, 0, SCREEN_W, SCREEN_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glFrustum(-1.0, -1.0, -1.0, -1.0, 1, 60.0);
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1, 60.0);


    /* Set culling mode - not that we have anything to cull */
    // glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW); // ?? GL_CCW

    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    allegro_gl_set_allegro_mode();

//textout_ex(screen, font, "Hello World!", 50, 100, makecol(255, 0, 0), -1);


    for (int i=0; i<100; ++i) {
        drawGLScene();
        allegro_gl_flip(); // <--------------------
        rest(50);
    }

//	bpp = 32; // colors

//	set_color_depth(bpp);
//    int ret = set_gfx_mode(GFX_OPENGL_WINDOWED, 1280, 720, 0, 0);
//	if (ret != 0) {
//		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
//		allegro_message("Error starting graphical mode (bpp = %d) bit graphics mode\nError was:%s\n", bpp, allegro_error);
//		std::string errmsg("Error: Can not start allegro graphics library, allegro_error=" + std::string(allegro_error));		std::cerr<<errmsg<<std::endl;
//		throw std::runtime_error(errmsg);
//	}
//}
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

