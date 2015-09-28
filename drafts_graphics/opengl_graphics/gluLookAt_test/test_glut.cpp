// compilation: g++ test_glut.cpp -o test_glut.o -lalleggl -lGL -lglut &&  setfattr -n user.pax.flags -v "m" ./test_glut.o  && ./test_glut.o

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <allegro.h>
#include <alleggl.h>
#include <iostream>
#include <cmath>

float angle = 0.0;
float zoom = 1.0;
float step_z = -5.0;
float camera_x = 0.0;
float camera_z = 0.0;
float view_angle = 3.0;
 
void InitAllegro() {

	allegro_init();
	install_allegro_gl();
	install_keyboard();
	install_timer();
}
void InitGL() {
	allegro_gl_set(AGL_DOUBLEBUFFER, 1);
	allegro_gl_set(AGL_WINDOWED, TRUE);
	allegro_gl_set( AGL_COLOR_DEPTH, 32 );
	allegro_gl_set( AGL_Z_DEPTH, 8 );
	allegro_gl_set( AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_Z_DEPTH );

    if ( set_gfx_mode( GFX_OPENGL, 1280, 720, 0, 0 ) ) {
        allegro_message( "Cannot create window" );
		exit( 1 );
	}
	glEnable(GL_DEPTH_TEST);
	glViewport( 0, 0, SCREEN_W, SCREEN_H );
	glClearColor( 0, 0, 0, 0 );
}

void RenderScene() {
    // clear buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    // set up camera, coords of eye point, coords of reference point
    gluPerspective(45.0f, SCREEN_W/SCREEN_H, 1.0, 150.0);
    gluLookAt(camera_x-1,0,camera_z+3, camera_x+1,0,camera_z-3, 0,1,0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glTranslatef(camera_x,-1,camera_z);
    glRotatef(angle,0.0,1.0,0.0);
    glTranslatef(-camera_x,1, -camera_z);

    // draw wireSphere in place of camera
    glPushMatrix();
    glTranslatef(camera_x,-1,camera_z);
    glColor3f(0,0,1);
    glutWireSphere(0.2,30,30);
    glTranslatef(-camera_x,1,-camera_z);
    glPopMatrix();

    //glTranslatef( 0.0f, 0.0f, step_z );

    glPushMatrix();
    glTranslatef( 0.0f, 0.0f, step_z );
    // draw quads
    glBegin( GL_QUADS );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
        glVertex3f(  1.0f, -1.0f, 0.0f );
        glVertex3f(  1.0f,  1.0f, 0.0f );
        glVertex3f( -1.0f,  1.0f, 0.0f );
    glEnd();

    glBegin( GL_QUADS );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, -1.0f );
        glVertex3f(  1.0f, -1.0f, -1.0f );
        glVertex3f(  1.0f,  1.0f, -1.0f );
        glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd();
    glPopMatrix();


    glPushMatrix();
    glTranslatef(-30,0.0f,step_z);
    glBegin( GL_TRIANGLES);
        glColor3f(1,1,0);
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(20,0.0f,step_z+90);
    glBegin( GL_TRIANGLES);
        glColor3f(0,1,1);
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(30,0.0f,step_z+30);
    glBegin( GL_TRIANGLES);
        glColor3f(0.5,0.5,0.5);
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glFlush();
    allegro_gl_flip();
}
int main(int argc, char* argv[])
{
	InitAllegro();
	InitGL();
    glutInit(&argc, argv);

	int i=0;
    while(i<10000)
	{
		if(key[KEY_Q]) angle+=0.1;
        if(key[KEY_E]) angle-=0.1;
        if(key[KEY_W]) step_z+=0.1;
        if(key[KEY_S]) step_z-=0.1;
        if(key[KEY_Z]) zoom+=0.1;
        if(key[KEY_X]) zoom-=0.1;
        if(key[KEY_UP]) camera_z-=0.1;
        if(key[KEY_DOWN]) camera_z+=0.1;
        if(key[KEY_RIGHT]) { camera_x+=0.1; view_angle*=0.3; std::cout << "camera_x " << camera_x << std::endl; }
        if(key[KEY_LEFT]) camera_x-=0.1;
		if(key[KEY_ESC]) exit(0);

		RenderScene();
		i++;
	}
	allegro_exit();
	remove_allegro_gl();
}
END_OF_MAIN();
