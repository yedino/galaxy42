#include "c_geometry.hpp"

t_geo_point::t_geo_point ()
:
	x(),
	y()
{
}

t_geo_point::t_geo_point (t_geo_r p_x, t_geo_r p_y) : x(p_x), y(p_y) { }

t_geo_point c_geometry::point_on_line_between_distance (t_geo_point A, t_geo_point B, t_geo_r dist) {
	t_geo_r relative_distance = dist / distance(A, B);
	t_geo_point T(A.x + (B.x - A.x) * relative_distance, A.y + (B.y - A.y) * relative_distance);
	return T;
}

t_geo_point c_geometry::point_on_line_between_part (t_geo_point A, t_geo_point B, t_geo_r part) {
	t_geo_point T(A.x + (B.x - A.x) * part, A.y + (B.y - A.y) * part);
	return T;
}

t_geo_r c_geometry::distance (t_geo_point A, t_geo_point B) {
	return sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}


