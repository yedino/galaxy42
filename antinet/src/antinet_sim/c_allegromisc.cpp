#include <stdexcept>
#include "c_allegromisc.hpp"


void alex_thick_line (BITMAP *bmp, float x1, float y1, float x2, float y2, float thickness, int color) {
	float len = hypotf(x2 - x1, y2 - y1);
	float dx = 0.5f * thickness * (y2 - y1) / len;
	float dy = 0.5f * thickness * -(x2 - x1) / len;
	if (len == 0)
		return;


	int tmp[8] = {(int)(x1 + dx), (int)(y1 + dy), (int)(x1 - dx), (int)(y1 - dy), (int)(x2 - dx), (int)(y2 - dy),
		(int)(x2 + dx), (int)(y2 + dy)};

	polygon(bmp, 4, tmp, color);
}


struct BITMAP *alex_load_png (AL_CONST char *filename, struct RGB *pal) {
	BITMAP *bmp = load_png(filename, pal);
	if (bmp == NULL) {
		std::cerr << "ERROR: Can not load file (" << filename << ")" << std::endl;
		throw std::runtime_error("Can not load file " + string(filename));
	}
	return bmp;
}

struct BITMAP *alex_load_png (const string &filename, struct RGB *pal) {
	return alex_load_png(filename.c_str(), pal);
}
