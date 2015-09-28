//g++ proba.cpp -o proba.o -lalleggl -lGL &&  setfattr -n user.pax.flags -v "m" ./proba.o  && ./proba.o

#include <GL/gl.h>
#include <allegro.h>
#include <alleggl.h>

float angle=0.0;
float zoom = 1.0;
 
void	InitAllegro()
{
	// Inicjalizacja allegro
	allegro_init();
	// Instalowanie AllegroGL:
	install_allegro_gl();
	// Instalowanie klawiatury
	install_keyboard();
	// Instalowanie timera
	install_timer();
}
void	InitGL()
{
	allegro_gl_set(AGL_DOUBLEBUFFER, 1);
	allegro_gl_set(AGL_WINDOWED, TRUE);
	// Ustawienia głębi kolorów
	allegro_gl_set( AGL_COLOR_DEPTH, 32 );
	allegro_gl_set( AGL_Z_DEPTH, 8 );
	allegro_gl_set( AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_Z_DEPTH );
	// Tworzenie okna 
	if ( set_gfx_mode( GFX_OPENGL, 800, 600, 0, 0 ) )
	{
		// Błąd tworzenia okna komunikat
		allegro_message( "Nie moge utworzyc okna" );
		exit( 1 );
	}
	glEnable(GL_DEPTH_TEST);
	// Ustawienia ekranu
	glViewport( 0, 0, SCREEN_W, SCREEN_H );
	// Kolor tła
	glClearColor( 0, 0, 0, 0 );
}

void RenderScene()
{
// Czyszczenie ekranu i bufora głebi
glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

glMatrixMode( GL_PROJECTION );
glLoadIdentity();
glFrustum( -1.0*zoom, 1.0*zoom, -1.0*zoom, 1.0*zoom, 1.5f, 20.0f );
glMatrixMode( GL_MODELVIEW );
glLoadIdentity();

glTranslatef( 0.0f, 0.0f, -5.0f );// Przesunięcie sceny w tył
glRotatef(angle,1.0,0.0,0.0);

// Renderowanie kwadratu
glBegin( GL_QUADS );

glColor3f( 1.0f, 0.0f, 0.0f );// Kolor wierzchołka
	glVertex3f( -1.0f, -1.0f, 0.0f );//Współrzedne wierzchołka
	glVertex3f(  1.0f, -1.0f, 0.0f );
	glVertex3f(  1.0f,  1.0f, 0.0f );
	glVertex3f( -1.0f,  1.0f, 0.0f );
glEnd();

glBegin( GL_QUADS );
	glColor3f( 0.0f, 1.0f, 0.0f );// Kolor wierzchołka
	glVertex3f( -1.0f, -1.0f, -1.0f );//Współrzedne wierzchołka
	glVertex3f(  1.0f, -1.0f, -1.0f );
	glVertex3f(  1.0f,  1.0f, -1.0f );
	glVertex3f( -1.0f,  1.0f, -1.0f );
glEnd();
 
// Oczyszczanie
//glFlush();
// Przerysowanie ekranu
allegro_gl_flip();
}
int main()
{
	InitAllegro();
	InitGL();

	int i=0;
	while(i<5000)
	{
		if(key[KEY_Q]) angle+=0.1;
		if(key[KEY_W]) zoom+=0.1;
		if(key[KEY_S]) zoom-=0.1;
		if(key[KEY_ESC]) exit(0);
		
		RenderScene();
		i++;
	}
	//Zwalnianie zasobów
	allegro_exit();
	remove_allegro_gl();
}
END_OF_MAIN();
