#ifndef C_GEOMETRY_HPP
#define C_GEOMETRY_HPP

#include "libs1.hpp"
//#include "c_geometry.hpp"


typedef double t_geo_r; // the precission used in our geometry. "r" as "real number"

/// @class A point in our geometry. With float (or more) precission
struct t_geo_point {
	t_geo_r x, y;

	t_geo_point ();

	t_geo_point (t_geo_r p_x, t_geo_r p_y);
};

class c_geometry {
public:

	/// A(x1,y1)----T(xt,yt)--------------B(x2,y2) find target point T that on the line between the given two points A,B at distance dist from point A in direction of B
	/// (can be "before" A, or after "B" too). T-----A---B for (dist < 0)    A---B-----T for (dist > distance(A,B))
	/// if A==B, then the returned point T==A
	static t_geo_point point_on_line_between_distance (t_geo_point A, t_geo_point B, t_geo_r dist);

	/// the same as point_on_line_between_distance(), but we give a number how far along from A to B the point is.
	/// for part=0.0 the point T will be placed on A, for 1.0 it will be on B, for 0.5 in middle etc. Can be any real number <0 and >1 too.
	static t_geo_point point_on_line_between_part (t_geo_point A, t_geo_point B, t_geo_r part);

	static t_geo_r distance (t_geo_point A, t_geo_point B);

	///
	/// Allegro 4.4 -- calculate points for octagon with given side and draw it
	///
	static void draw_octagon (BITMAP *bmp, int a, int x, int y, int color, int border = 1);
	
	// TODO @opengl - here add new stuff
};


#endif // include guard

