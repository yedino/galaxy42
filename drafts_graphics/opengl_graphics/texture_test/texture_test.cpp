// compilation: g++ test.cpp loadpng.cpp -o test.o -lz -lpng -lalleggl -lglut &&  setfattr -n user.pax.flags -v "m" ./test.o  && ./test.o

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <allegro.h>
#include <alleggl.h>
#include <iostream>
#include <cmath>
#include <stdio.h>

#include "loadpng.hpp"

float angle = 0.0;
float zoom = 1.0;
float step_z=-5.0;
float camera_x = 0.0;
float camera_y = 0.0;
float camera_z = -5.0;
float view_angle = 3.0;

BITMAP *texture;
GLuint texture_id;

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

    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);        // Enables Depth Testing

    glViewport( 0, 0, SCREEN_W, SCREEN_H );
    glClearColor( 0, 0, 0, 0 );
}

void DrawGround() {
    glPushMatrix();
    for(int i=0; i<50; i++) {
        for(int j=0; j<50; j++) {

            if( (i+j)%2 ) glColor3f(0,0,0);
            else glColor3f(1,1,1);

            glBegin(GL_QUADS);
            glVertex3f(-25+i, -1, -25+j);
            glVertex3f(-25+i+1, -1, -25+j);
            glVertex3f(-25+i+1, -1, -25+j+1);
            glVertex3f(-25+i, -1, -25+j+1);
            glEnd();
        }
    }
    glPopMatrix();
}

void RenderScene() {
    // clear buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    // set up camera, coords of eye point, coords of reference point
    gluPerspective(45.0f, (float)SCREEN_W/(float)SCREEN_H, 1.0, 150.0);
    gluLookAt (camera_x - 1*sin(angle),0,camera_z + 1*cos(angle),camera_x + 5*sin(angle),camera_y,camera_z - 5*cos(angle),0,1,0);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glTranslatef(camera_x,0,camera_z);
    glRotatef(angle,0,1,0);

    // draw sphere in place of camera
    glPushMatrix();
    glTranslatef(0,-0.5,0);
    glColor3f(0,0,1);
    glutSolidSphere(0.1,30,30);
    glPopMatrix();

    glPushMatrix();
    glTranslatef( camera_x, 0.0f, camera_z );
    DrawGround();

    glEnable(GL_TEXTURE_2D);            // Enables using 2d textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // draw cube
    glBegin( GL_QUADS );        //front
        //glColor3f( 1.0f, 0.0f, 0.0f );
        glTexCoord2d(0,1); glVertex3f( -1.0f, -1.0f, 0.0f );
        glTexCoord2d(1,1); glVertex3f(  1.0f, -1.0f, 0.0f );
        glTexCoord2d(1,0); glVertex3f(  1.0f,  1.0f, 0.0f );
        glTexCoord2d(0,0); glVertex3f( -1.0f,  1.0f, 0.0f );
    glEnd();


    glBegin( GL_QUADS );        //right
       // glColor3f( 0.5f, 0.2f, 0.1f );
        glTexCoord2d(0,1); glVertex3f( 1.0f, -1.0f, 0.0f );
        glTexCoord2d(1,1); glVertex3f(  1.0f, -1.0f, -1.0f );
        glTexCoord2d(1,0); glVertex3f(  1.0f,  1.0f, -1.0f );
        glTexCoord2d(0,0); glVertex3f( 1.0f,  1.0f, 0.0f );
    glEnd();

    glBegin( GL_QUADS );        //back
        glColor3f( 0.0f, 1.0f, 0.0f );
        glTexCoord2d(0,1); glVertex3f( -1.0f, -1.0f, -1.0f );
        glTexCoord2d(1,1); glVertex3f(  1.0f, -1.0f, -1.0f );
        glTexCoord2d(1,0); glVertex3f(  1.0f,  1.0f, -1.0f );
        glTexCoord2d(0,0); glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd();


    glBegin( GL_QUADS );        //down
        glColor3f( 0.3f, 0.8f, 0.3f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
        glVertex3f(  1.0f, -1.0f, 0.0f );
        glVertex3f(  1.0f,  -1.0f, -1.0f );
        glVertex3f( -1.0f,  -1.0f, -1.0f );
    glEnd();

    glBegin( GL_QUADS );        //up
        glColor3f( 0.0f, 0.5f, 0.2f );
        glVertex3f( -1.0f, 1.0f, 0.0f );
        glVertex3f(  1.0f, 1.0f, 0.0f );
        glVertex3f(  1.0f,  1.0f, -1.0f );
        glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd();

    glBegin( GL_QUADS );        //left
        glColor3f( 0.3f, 0.3f, 0.3f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
        glVertex3f(  -1.0f, -1.0f, -1.0f );
        glVertex3f(  -1.0f,  1.0f, -1.0f );
        glVertex3f( -1.0f,  1.0f, 0.0f );
    glEnd();



    glDisable(GL_BLEND);
	  glDisable(GL_TEXTURE_2D);       // disable texture 2d when we don't use it


    glPopMatrix();

    // draw triangles in some places
    glPushMatrix();
    glTranslatef( camera_x-3, 0.0f, camera_z+10 );
    glBegin( GL_TRIANGLES);
        glColor3f(1,1,0);
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(camera_x+7,0.0f,camera_z+9);
    glBegin( GL_TRIANGLES);
        glColor3f(0.0f,1.0f,1.0f);
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(camera_x-15,0.0f,camera_z-3);
    glBegin( GL_TRIANGLES);
        glColor3f( 0.5,0.5,0.5 );
        glVertex3f(-1,-1,0);
        glVertex3f(1,-1,0);
        glVertex3f(1,1,0);
    glEnd();
    glPopMatrix();

    glFlush();
    allegro_gl_flip();
}

int main(int argc, char* argv[]) {
    InitAllegro();
    InitGL();
    glutInit(&argc, argv);

    // texture = load_png("server_96x96.png",NULL);
    texture = load_png("5140603528_533e70da06_b.png",NULL);
    std::cout << (void*)texture << std::endl;

    //allegro_gl_use_mipmapping(TRUE);
    //texture_id = allegro_gl_make_masked_texture(texture);
    texture_id = allegro_gl_make_texture_ex(AGL_TEXTURE_HAS_ALPHA, texture, -1);

    while(true) {
        if(key[KEY_UP]) {
            camera_z+=cos(angle)*0.1;
            camera_x-=sin(angle)*0.1;
        }
        if(key[KEY_DOWN]) {
            camera_z-=cos(angle)*0.1;
            camera_x+=sin(angle)*0.1;
        }
        if(key[KEY_RIGHT]) angle+=0.01;
        if(key[KEY_LEFT]) angle-=0.01;

        if(key[KEY_W]) camera_y+=0.1;
        if(key[KEY_S]) camera_y-=0.1;

        if(key[KEY_ESC]) exit(0);

        RenderScene();
    }

    destroy_bitmap(texture);
    allegro_exit();
    remove_allegro_gl();
}
END_OF_MAIN();
