g++ texture_test.cpp loadpng.cpp -lGL -lGLU -lglut $(allegro-config --libs --cppflags) -lalleggl -lpng -lX11 && ./a.out

