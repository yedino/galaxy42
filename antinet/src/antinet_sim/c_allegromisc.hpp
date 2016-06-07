#ifndef C_ALLEGROMISC_HPP
#define C_ALLEGROMISC_HPP

#include "libs1.hpp"
#include "c_skel.hpp"

class c_allegromisc { };

// like line, but allows to specify the thickness
void alex_thick_line (BITMAP *bmp, float x, float y, float x_, float y_, float thickness, int color);

// loads bitmap, but throws on error. If returns then it always returns valid bitmap
struct BITMAP *alex_load_png (AL_CONST char *filename, struct RGB *pal);

struct BITMAP *alex_load_png (const string &filename, struct RGB *pal);

class c_al_bitmap final {
	public:
		BITMAP* m_bitmap;

		c_al_bitmap();
		c_al_bitmap(BITMAP* bitmap); ///< takes over a bitmap (owns it - will delete it later)
		c_al_bitmap(const c_al_bitmap & other)=delete;
		~c_al_bitmap();

		c_al_bitmap& operator=(BITMAP* bitmap); ///< takes over a bitmap (owns it - will delete it later)
		c_al_bitmap& operator=(const c_al_bitmap &other)=delete;

		BITMAP& operator*();
		const BITMAP& operator*() const;

		BITMAP* operator->();
		const BITMAP* operator->() const;

		operator BITMAP*();
		operator const BITMAP*() const;
};

ostream& operator<<(ostream& ostr, const c_al_bitmap & bmp);

#endif // include guard

