#include "c_object.hpp"
#include "c_geometry.hpp"
#include "c_gui.hpp"
#include "c_world.hpp"
#include "c_allegromisc.hpp"
#include <stdexcept>
#include "c_drawtarget_opengl.hpp"
#include "c_api_tr.hpp"

#include "use_opengl.hpp"

c_object::c_object (string name)
:
	s_font_allegl(nullptr),
	m_name(name),
	m_animframe(0)
{
}

void c_object::tick () { }

const string & c_object::get_name () const { return m_name; }

void c_object::print(std::ostream &oss) const
{
	oss << "[obj "<<m_name<<"]";
}

std::ostream& operator<<(std::ostream &oss, const c_object &obj) 
{
	obj.print(oss);
	return oss;
}


void c_object::draw_allegro(c_drawtarget &drawtarget, c_layer &layer) {
	_dbg3("Drawing (allegro) object (base method used)");
}

void c_object::draw_opengl(c_drawtarget &drawtarget, c_layer &layer) {
	_dbg3("Drawing (opengl) object (base method used)");
}

// ==================================================================

// @TODO rename to draw_allegro?
void c_wallet::draw (BITMAP *frame, int color, t_pos x, t_pos y) const {
	for (const auto &currency: m_currency) {
		std::string text(currency.first + " " + std::to_string(currency.second));
        textout_ex(frame, font, text.c_str(), x, y, color, 255);
		y -= 10;
	}
}

// ==================================================================

c_entity::c_entity (string name, t_pos x, t_pos y) : c_object(name), m_x(x), m_y(y) { }

/*
void c_entity::draw (const c_drawtarget &drawtarget, c_layer_opengl &layer, int color) const {
{
	m_x , m_y ...
	if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
		triangles ...
		glQuad
	}
	...
}
*/


t_pos c_entity::get_x(){return m_x;}
t_pos c_entity::get_y(){return m_y;}

double c_entity::get_distance (const c_entity &entity) {
	t_geo_point my_point(m_x, m_y);
	t_geo_point remote_point(entity.m_x, entity.m_y);
	return c_geometry::distance(my_point, remote_point);
}


void c_entity::draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any) {
	//std::cout << "TODO: c_entity::draw_opengl" << std::endl;

    auto layer = dynamic_cast<c_layer_opengl &> (layer_any);

    const auto & gui = * drawtarget.m_gui;
    const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
    glPushMatrix();

    if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
			// auto selected_object = gui.m_selected_object;


        if (m_selected) { // if I am the selected object
            //circle(frame, vx, vy, 50 - 5, makecol(255, 128, 32));
            glLineWidth(2.0);  //size of line
            glColor3f(1.0, 1.0, 0.0);
//            glBegin(GL_LINES);
//            glVertex3f(0.0, 0.0, 0.0);
//            glVertex3f(-1, 0, 0);
//            glEnd();
        }

        if (m_target) { // if I am the target object
            //circle(frame, vx, vy, 50 - 15, makecol(104, 71, 79));
            glLineWidth(1.5);  //size of line
            glColor3f(0.0, 1.0, 0.0);

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.05*cos(angle)*gui.camera_zoom;
                float y = 0.1*sin(angle)*gui.camera_zoom;
                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }

        if (m_source) {
            //circle(frame, vx, vy, 50 - 15, makecol(246, 83, 86));
            glLineWidth(1.5);  //size of line
            glColor3f(0.0, 1.0, 0.0);

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.05*cos(angle)*gui.camera_zoom;
                float y = 0.1*sin(angle)*gui.camera_zoom;

                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }

    }
    if (layer.m_layer_nr == e_layer_nr_gui) {
        if (m_selected) { // if I am the selected object
            glLineWidth(1.0);
            glColor3f(1.0, 0.0, 0.0);

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.07*cos(angle)*gui.camera_zoom;
                float y = 0.14*sin(angle)*gui.camera_zoom;

                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }
    }

    if (layer.m_layer_nr == e_layer_nr_object) {

            glBegin(GL_POLYGON);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.05*cos(angle)*gui.camera_zoom;
                float y = 0.1*sin(angle)*gui.camera_zoom;
                glVertex3f(x,y,0.0f);
            }
            glEnd();

        //std::cout << "DEBUG5" << std::endl;
//		line(frame, vx - 2, vy - 2, vx + 2, vy + 2, color);
//		line(frame, vx - 2, vy + 2, vx + 2, vy - 2, color);
//		circle(frame, vx, vy, 10, color);
    }
    glPopMatrix();

}

void c_entity::draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any) {
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	const auto & gui = * drawtarget.m_gui;
	const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position

	if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
		auto selected_object = gui.m_selected_object; // iterator
		_UNUSED(selected_object);

        if (m_selected) { // if I am the selected object
			circle(frame, vx, vy, 50 - 5, makecol(255, 128, 32));
		}

//        if (target_object) { // if I am the target object
//            circle(frame, vx, vy, 50 - 15, makecol(104, 71, 79));
//        }

//        if (source_object) {
//            circle(frame, vx, vy, 50 - 15, makecol(246, 83, 86));
//        }


        if (m_target) { // if I am the target object
			circle(frame, vx, vy, 50 - 15, makecol(104, 71, 79));
		}

        if (m_source) {
            circle(frame, vx, vy, 50 - 15, makecol(246, 83, 86));
        }
//		if (this == source_object.get()) {
//			circle(frame, vx, vy, 50 - 15, makecol(246, 83, 86));
//		}
	}
	if (layer.m_layer_nr == e_layer_nr_gui) {
		//auto selected_object = gui.m_selected_object;
		if (m_selected) { // if I am the selected object
			//_dbg3("draw select");
			const int r1 = 50 - 8, r2 = 50;
			const auto col1 = makecol(192, 192, 192), col2 = makecol(255, 128, 32);
			int nr = 0;
			for (double angle = 0; angle < M_PI * 2; angle += (M_PI * 2) / 16) {
				++nr;
				line(frame,
				     vx + sin(angle) * r1,
				     vy + cos(angle) * r1, vx + sin(angle) * r2, vy + cos(angle) * r2, (nr % 2 ? col1 : col2));
			}
		}
	}

	int color = makecol(255,128,0); // color1
	if (layer.m_layer_nr == e_layer_nr_object) {
		line(frame, vx - 2, vy - 2, vx + 2, vy + 2, color);
		line(frame, vx - 2, vy + 2, vx + 2, vy - 2, color);
		circle(frame, vx, vy, 10, color);
	}
    if (layer.m_layer_nr == e_layer_nr_object_extra) {
		textout_ex(frame, font, m_name.c_str(), vx - 20, vy - 35, color, -1);

	}
}

// ==================================================================

// ==================================================================


unsigned int cjddev_detail_next_price (unsigned int current) {
	unsigned int prices[] = {1, 5, 10, 20, 25, 50, 100, 200, 1000, 10000};
	const size_t tab_size = sizeof(prices) / sizeof(prices[0]);
	_obvious(tab_size > 0);
	size_t next_i = 0;
	for (size_t i = 0; i < sizeof(prices) / sizeof(prices[0]); ++i) {
		if (prices[i] == current) {
			next_i = i + 1;
		}
	}
	if (next_i == tab_size)
		next_i = 0;
	_assert(next_i < tab_size);
	return prices[next_i];
}

void cjddev_detail_next_price_inc (unsigned int &current) {
	current = cjddev_detail_next_price(current);
}

void draw_string(string tmp_string, c_drawtarget &drawtarget, c_layer &layer_any) {

    const auto & gui = * drawtarget.m_gui;
    float m_size = 0.005*gui.camera_zoom;
    float offset = 0.01*gui.camera_zoom;
    for(unsigned int i=0; i<tmp_string.length(); i++){
    //_dbg1("ASCII A: " << (int)A );
    float x_position = ((((int)tmp_string[i]-32)%10)/10.);
    float y_position = 1-((((int)tmp_string[i]-32)-(((int)tmp_string[i]-32)%10))/100.);

//    _dbg1("x_position: " << x_position);
//    _dbg1("y_position: " << y_position);

    glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c_bitmaps::get_instance().m_node->w, c_bitmaps::get_instance().m_node->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture (GL_TEXTURE_2D, c_bitmaps::get_instance().m_bitmap_font1_opengl);   //init a texture
        glBegin( GL_QUADS );                                /* Drawing Using Quads       */

        glTexCoord2f (x_position, y_position);
        glVertex3f(  -1.0f*m_size+0.01+(i*offset),  2.0f*m_size-0.1,  0.0f );   /* Left top       */
        glTexCoord2f (x_position+0.09, y_position);
        glVertex3f( 1.0f*m_size+0.01+(i*offset), 2.0f*m_size-0.1,  0.0f );      /* Right top      */
        glTexCoord2f (x_position+0.09, y_position-0.09);
        glVertex3f(  1.0f*m_size+0.01+(i*offset), -2.0f*m_size-0.1,  0.0f );    /* Right bottom   */
        glTexCoord2f (x_position, y_position-0.09);
        glVertex3f( -1.0f*m_size+0.01+(i*offset), -2.0f*m_size-0.1, 0.0f);      /* Left bottom    */

        glEnd( );                                           /* Finished Drawing The Quads */
        glBindTexture(GL_TEXTURE_2D, 0);   // texture
        glDisable(GL_BLEND);
    }
}

// ==================================================================

