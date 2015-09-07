#include "c_gui.hpp"

c_gui::c_gui() {
	move_to_home();
}

void c_gui::move_to_home() {
	camera_x=0;
	camera_y=0;
	camera_z=20;
}

int c_gui::view_x(int x) const {
	return x*2 - camera_x;
}

int c_gui::view_y(int y) const {
	return y*2 - camera_y;
}


int c_gui::view_x_rev(int x) const {
	return x*2 + camera_x;
}

int c_gui::view_y_rev(int y) const {
	return y*2 + camera_y;
}

