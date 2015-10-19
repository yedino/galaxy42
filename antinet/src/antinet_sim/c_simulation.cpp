#include "c_simulation.hpp"
#include "c_gui.hpp"
#include <stdlib.h>
#include "use_opengl.hpp"

#include "c_drawtarget_opengl.hpp"

unsigned int g_max_anim_frame = 10;

c_simulation::c_simulation (t_drawtarget_type drawtarget_type) 
: 
	m_goodbye(false), m_frame_number(0), 
	m_drawtarget_type( drawtarget_type ),
	m_frame(nullptr), 
	m_screen( m_drawtarget_type == e_drawtarget_type_allegro ? screen : nullptr),
	smallWindow(nullptr),
	m_drawtarget(nullptr)
{
	_note("Creating simulation, mode: " << t_drawtarget_type_to_string( m_drawtarget_type ) << ". The allegro screen is at " << (void*)m_screen );
}

c_simulation::~c_simulation () {
	_note("Destructing the simulation");

	if (m_frame) {
		destroy_bitmap(m_frame);
		m_frame = nullptr;
	}
	
	if (smallWindow) {
		destroy_bitmap (smallWindow);
    smallWindow = nullptr;
	}

	_note("Destructing the simulation - done");
}

void c_simulation::init () {
	//this->close_button_pressed = false;
	_note("Init. We are using m_screen=" << m_screen);

	if (m_drawtarget_type == e_drawtarget_type_allegro) {
		_note("Creating the allegro frame buffer for m_screen="<<m_screen);
		_note("Screen size is: " << m_screen->w << "*" << m_screen->h );
		m_frame = create_bitmap(m_screen->w, m_screen->h);
		_note("Done, the m_frame="<<m_frame);

		smallWindow = create_bitmap(m_screen->w,m_screen->h); 
	}
	if (m_drawtarget_type == e_drawtarget_type_opengl) {
		_note("Using the opengl in simulation");
		// TODO @opengl load here some textures and other general things
	}

	_note("Simulation will create the world");
	m_world = make_unique<c_world>();
	m_world->add_test();
	//m_world->add_i_objects(100);

	_note("Simulation created m_world=" << m_world.get());
	_note("Simulation is ready");
}

std::string cjddev_detail_random_name () {
	return "newnode";
}

std::string cjddev_detail_random_addr () {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
	return std::to_string(distribution(generator));
}


void c_simulation::main_loop () {
	//	PALETTE palette;
	//BITMAP *img_bgr = load_bitmap("dat/bgr-bright.tga", NULL); // TODO:
    s_font_allegl.reset (allegro_gl_convert_allegro_font(font,AGL_FONT_TYPE_TEXTURED,500.0), [](FONT *f){allegro_gl_destroy_font(f);});

    for(auto &obj : m_world->m_objects) {
        obj->set_font(s_font_allegl);
    }
	int viewport_x = 0, viewport_y = 0;

	//show_mouse(m_screen);

	set_close_button_callback(c_close_button_handler);

	bool print_connect_line = false;
	bool start_simulation = false;
	bool simulation_pause = true;
	shared_ptr<c_cjddev> connect_node;
	std::chrono::steady_clock::time_point last_click_time =
		std::chrono::steady_clock::now() - std::chrono::milliseconds(1000);

	m_gui = make_shared<c_gui>();


	// prepare drawtarget surface to draw to
	switch (m_drawtarget_type) {
		case e_drawtarget_type_allegro:
			m_drawtarget = make_shared<c_drawtarget_allegro>(m_frame); 
		break;
		case e_drawtarget_type_opengl:
			m_drawtarget = make_shared<c_drawtarget_opengl>(); 
		break;
		default:
			_erro("Warning: unsupported drawtarget");
	}

	m_drawtarget->m_gui = m_gui;



	//	bool allegro_keys_any_was=false; // is any key pressed right now (for key press/release)
	long loop_miliseconds = 0;
	long unsigned int frame_checkpoint = 0; /// needed for speed control (without world_draw manipulate in spacetime!)
	_UNUSED(frame_checkpoint);

	bool use_input_allegro = true; // always for now.  input from Allegro
	bool use_draw_allegro = m_drawtarget_type == e_drawtarget_type_allegro; // draw in allegro
	bool use_draw_opengl = m_drawtarget_type == e_drawtarget_type_opengl; // draw in opengl


	_note("Entering main simulation loop");

	// The main drawing is done inside this loop.
	
	///@see rendering.txt/[[drawing_main]]
    float view_angle = 0.0;
    //float camera_offset = 1.0;

    float zoom = 1.0;
    float camera_step_z=-11.0;
    // === main loop ===
    while (!m_goodbye && !close_button_pressed) {

		auto start_time = std::chrono::high_resolution_clock::now();

		// --- process the keyboard/inputs ---
		if (use_input_allegro) {
				// TODO move this code here, but leave the variables in higher scope
		}

			poll_keyboard();
			auto allegro_keys = key;
			auto allegro_shifts = key_shifts;
			//		bool allegro_keys_any_is=false;
			//		for (size_t i=0; i<sizeof(allegro_keys)/sizeof(allegro_keys[0]); ++i) allegro_keys_any_is=true;
			// the direct raw position
			const int allegro_mouse_x = mouse_x;
			const int allegro_mouse_y = mouse_y;
			const int allegro_mouse_b = mouse_b; // buttons

			// the position in display port GUI
			const int gui_mouse_x = allegro_mouse_x; 
			const int gui_mouse_y = allegro_mouse_y;
			const int gui_mouse_b = allegro_mouse_b; // buttons
			
			// the position in the world coordinates
			const int gui_cursor_x = m_gui->view_x_rev(gui_mouse_x);
			const int gui_cursor_y = m_gui->view_y_rev(gui_mouse_y);
			const int gui_cursor_z = 0; // m_gui->view_z_rev(gui_mouse_z);
			
			_UNUSED(gui_mouse_b);
			_UNUSED(gui_cursor_x);
			_UNUSED(gui_cursor_y);
			_UNUSED(gui_cursor_z);

            //_dbg1("mouse_x mouse_y: " << gui_mouse_x << " " << gui_mouse_y);

			int allegro_char = 0;
			if (keypressed()) {
				allegro_char = readkey();
			}
		// end of input

		// draw background of frame
		if (use_draw_allegro) {
			clear_to_color(m_frame, makecol(0, 128, 0));
            blit(c_bitmaps::get_instance().m_background, m_frame, 0, 0, viewport_x, viewport_y, c_bitmaps::get_instance().m_background->w, c_bitmaps::get_instance().m_background->h);
		}
        if (use_draw_opengl) {
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //glDisable(GL_DEPTH_TEST);      // ??? Enables Depth Testing
            //glEnable(GL_DEPTH_TEST);
            //glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
            //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
             //glLoadIdentity();
            // glTranslatef(m_gui->camera_x, m_gui->camera_y,camera_offset);

            // minimum and maximum value for zoom in/out and rotate the scene
//            if (camera_offset >= 10.0) camera_offset = 10.0;
//            if (camera_offset <= 0.5) camera_offset = 0.5;
            if (view_angle >= 70) view_angle = 70;
            if(view_angle <= 0) view_angle = 0;

            if( zoom <= 0.1 ) zoom = 0.1;  //because of glFrustum -> when left=right, or bottom=top there's error GL_INVALID_VALUE, so we can't multiply e.g left,right by 0

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-1.0*zoom, 1.0*zoom, -1.0*zoom, 1.0*zoom, 1.0,60.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            //glTranslatef(0.0f,0.0f,-11.0);
            glTranslatef(0.0f,0.0f,camera_step_z);
            glRotatef(-view_angle, 1,0,0);
            glScalef(10,10,10);

            // drawing backgound
            glPushMatrix();
            //glScalef(1,1,1);
            glBindTexture(GL_TEXTURE_2D,c_bitmaps::get_instance().m_background_opengl);
            glEnable(GL_BLEND);
            //float q=1.0/zoom;
            float q=1.0;
            glBegin(GL_QUADS);
                glTexCoord2f(0,q); glVertex3f(-1.0f,1.0f, 0.0f);
                glTexCoord2f(q,q); glVertex3f(1.0f,1.0f, 0.0f);
                glTexCoord2f(q,0); glVertex3f(1.0f,-1.0f,0.0f);
                glTexCoord2f(0,0); glVertex3f(-1.0f,-1.0f,0.0f);
            glEnd();
            glDisable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, 0);   // texture
            glPopMatrix();
		}
		
		// clear additional things
		if (use_draw_allegro) {
			clear_to_color(smallWindow, makecol(128, 128, 128));
		}

		// main controll keys
		if (allegro_keys[KEY_ESC]) {
			_note("User exits the simulation from user interface");
			m_goodbye = true;
		}

		if ((allegro_char & 0xff) == 'n' && !start_simulation) {
			std::cout << "ADD " << std::endl;
			_warn("THIS CODE IS NOT IMPLEMENTED NOW");
			/*
			m_world->m_objects.push_back(
				make_shared<c_cjddev>(
					cjddev_detail_random_name(), 
					// gui_mouse_x, gui_mouse_y,
					gui_cursor_x, gui_cursor_y,
					cjddev_detail_random_addr()));
					*/
		}

        if(allegro_keys[KEY_F1]){
            //auto ptr = get_move_object(gui_mouse_x,gui_mouse_y);


						//  TODO  -in allegro?   -not entire screen?    
						/*
            try{
                if(ptr != NULL){
                    int col_num =0;
                    textout_ex(smallWindow, font, ptr->get_name().c_str(), 0, 0, makecol(0, 0, 255), -1);

                        if(c_cjddev* tmp = dynamic_cast<c_cjddev *>(ptr.get()) ){
                            char* addr =(char *) malloc(45);
                             sprintf(addr,"address: %ld",tmp->get_address());
                             textout_ex(smallWindow, font,addr , 10, col_num+=10, makecol(0, 0, 255), -1);
                             sprintf(addr,"neighbors: %d",(int)tmp->get_neighbors().size());
                             textout_ex(smallWindow, font,addr , 10, col_num+=10, makecol(0, 0, 255), -1);
                             sprintf(addr,"waitng: %d",(int)tmp->num_of_wating());
                             textout_ex(smallWindow, font,addr , 10, col_num+=10, makecol(0, 0, 255), -1);

//                    		textout_ex(smallWindow, font, ptr->get_name().c_str(), 0, 0, makecol(0, 0, 255), -1);{
                            free (addr);
                        }

												if (use_draw_allegro) { 
													// draw the information window
                        	blit (smallWindow,m_frame,0,0,m_frame->w-200,m_frame->h-200,screen->w/8, screen->h/4);
												}

                    }
            }
						catch(...) {}
						*/

//            std::cout<<ptr->get_name().c_str()<<std::endl;
        }
				

        if(allegro_keys[KEY_F2]){
//            BITMAP* screen = gui_get_screen();
          int m_x =0;
          int m_y =0;
          static unsigned int num =0;
            if(num>=m_world->m_objects.size()){
                num=0;
            }
           try{

//                auto obj = m_world->m_objects.at(0);
//                m_x = m_world->m_objects.at(num)->get_x() - (screen->w/2);
//                m_y = m_world->m_objects.at(num)->get_y() - (screen->h/2);
                  m_x =  m_world->m_objects.at(num)->get_x()*m_gui->camera_zoom - (allegro_mouse_x);
                  m_y =  m_world->m_objects.at(num)->get_y()*m_gui->camera_zoom - (allegro_mouse_y);


                //    std::cout<< screen->w<<" "<<screen->h<<" "<<screen->x_ofs<<" "<<screen->y_ofs<<std::endl;

                    m_gui->camera_x = m_x ;
                    m_gui->camera_y = m_y;


                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    num++;
            }catch(...)
            {

            }

        }


		// animation & tick
		if (m_frame_number % g_max_anim_frame == 0 && !simulation_pause) {
			frame_checkpoint = m_frame_number;
			m_world->tick(); // <===
		}


		// === main user interface ===

		// the mode
		typedef enum { e_mode_node, e_mode_camera } t_mode;
		t_mode mode;
		mode = e_mode_node; // by default we will move/edit etc the node (or entityt)
		if (allegro_shifts & KB_SHIFT_FLAG) mode = e_mode_camera; // with key SHIFT we move camera instea

		// mode: camera movement etc
		if (mode == e_mode_camera) {
			if (allegro_keys[KEY_LEFT]) m_gui->camera_x -= 10;
			if (allegro_keys[KEY_RIGHT]) m_gui->camera_x += 10;
            if (allegro_keys[KEY_UP]) m_gui->camera_y -= 10;
			if (allegro_keys[KEY_DOWN]) m_gui->camera_y += 10;

			const double zoom_speed = 1.1;
            if (allegro_keys[KEY_PGUP]) m_gui->camera_zoom *= zoom_speed;
			if (allegro_keys[KEY_PGDN]) m_gui->camera_zoom /= zoom_speed;
		}

        // rotate and zoom in/out the scene
        if(allegro_keys[KEY_Z]) {
            //camera_offset+=0.1;
            zoom+=0.1;
            _dbg1("zoom: " << zoom);
        }

        if(allegro_keys[KEY_X]) {
            //camera_offset-=0.1;
            zoom-=0.1;
            _dbg1("zoom: " << zoom);
        }
        if(allegro_keys[KEY_C]) {
            view_angle+=1.0;
            //farVal+=10;
            _dbg1("view_angle: " << view_angle);
        }
        if(allegro_keys[KEY_V]) {
            view_angle-=1.0;
            _dbg1("view_angle: " << view_angle);
        }
        if(allegro_keys[KEY_Q]) {
            camera_step_z+=0.1;
        }
        if(allegro_keys[KEY_W]) {
            camera_step_z -= 0.1;

            if(camera_step_z <= -11.0) camera_step_z=-11.0;
        }

		// === text debug on screen ===

		string mouse_pos_str = std::to_string(gui_mouse_x) + " " + std::to_string(gui_mouse_y);
		string fps_str = "fps ???";
		if (loop_miliseconds != 0) {
			fps_str = "fps: " + std::to_string(1000 / loop_miliseconds);
		}

		const int txt_h = 12; // line height (separation between lines)
		int txt_x = 10, txt_y = 10; // starting position of text

		if (use_draw_allegro) {

			string pck_speed_str = "sending packets speed - " + std::to_string(450 - g_max_anim_frame);
			textout_ex(m_frame, font, mouse_pos_str.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
			textout_ex(m_frame, font, fps_str.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
			textout_ex(m_frame, font, ("Frame nr.: " +
																 std::to_string(m_frame_number)).c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);

			textout_ex(m_frame, font, pck_speed_str.c_str(), 100, 10, makecol(0, 0, 255), -1);

					if(allegro_keys[KEY_H])
			{
				int tex_y = 10;
				int lineh = 10;
				textout_ex(m_frame, font, "s - start", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "p - pause", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "f - send FTP", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "t - select target", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "r - select source", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "d - remove node", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "n - add node", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "enter/esc - exit", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "Arrows: move selected node", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
				textout_ex(m_frame, font, "SHIFT-Arrows: move the camera", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
							textout_ex(m_frame, font, "SHIFT-PageUp/Down: zimm in/out", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
							textout_ex(m_frame, font, "F1: info about node", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
							textout_ex(m_frame, font, "F2: next node", 1140, tex_y+=lineh, makecol(0, 0, 255), -1);
					} else{

							textout_ex(m_frame, font, "h - help", 1140, 30, makecol(0, 0, 255), -1);
					}
		}
		if (use_draw_opengl) {
			// TODO @opengl
            //textout_ex(m_frame, font, mouse_pos_str.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
            float offset = 0.03;
            float tex_y = 0.9;
            float tex_x = 0.7;
            string pck_speed_str = "sending packets speed - " + std::to_string(450 - g_max_anim_frame);
            glColor4f(0.0,0.0,1.0,0.0);
            //glScalef(0.2f,0.2f,0.2f);
            //glLoadIdentity();
            glPushMatrix();
            glEnable(GL_BLEND);
            allegro_gl_printf_ex(s_font_allegl.get(), -0.9, tex_y, 0.0, mouse_pos_str.c_str());
            allegro_gl_printf_ex(s_font_allegl.get(), -0.9, tex_y-=offset, 0.0, fps_str.c_str());
            allegro_gl_printf_ex(s_font_allegl.get(), -0.9, tex_y-=offset, 0.0, ("Frame nr.: " + std::to_string(m_frame_number)).c_str());
            allegro_gl_printf_ex(s_font_allegl.get(), -0.7, 0.97, 0.0, pck_speed_str.c_str());
            glDisable(GL_BLEND);
            glPopMatrix();

            if(allegro_keys[KEY_H]) {
                _dbg1("KEY_H - opengl");
                //glLoadIdentity();
                glPushMatrix();
                glEnable(GL_BLEND);
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y, 0.0,"s - start");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"p - pause");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"f - send FTP");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"t - select target");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"r - select source");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"d - remove node");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"n - add node");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"enter/esc - exit");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"Arrows: move selected node");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"SHIFT-Arrows: move the camera");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"SHIFT-PageUp/Down: zoom in/out");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"F1: info about node");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y-=offset, 0.0,"F2: next node");
                glDisable(GL_BLEND);
                glPopMatrix();

            } else {
                //glLoadIdentity();
                glPushMatrix();
                glEnable(GL_BLEND);
                //allegro_gl_printf(s_font_allegl.get(), 0.8, 0.9, 0.0,0xFF0000,"h - help");
                allegro_gl_printf_ex(s_font_allegl.get(), tex_x, tex_y, 0.0,"h - help");
                //glBlendFunc(GL_ONE, GL_ZERO);
                glDisable(GL_BLEND);
                glPopMatrix();
            }
		}


		//_dbg3("m_world->m_objects.size(): " << m_world->m_objects.size());
		//_dbg3("get_move_object ret: " << get_move_object(gui_mouse_x, gui_mouse_y));
		int move_object_index = get_move_object(gui_mouse_x, gui_mouse_y); ///< -1 if 'empty'

		if (move_object_index != -1) { // working with selected object
			m_gui->m_selected_object = m_world->m_objects.begin();
			std::advance(m_gui->m_selected_object, move_object_index);
			auto selected_object = m_gui->m_selected_object;
			(*selected_object)->m_selected = true;
			c_entity *selected_object_raw = dynamic_cast<c_entity *>((*selected_object).get());
			c_osi2_switch *selected_switch = dynamic_cast<c_osi2_switch *>((*selected_object).get());
			
			if (allegro_keys[KEY_K]) {
				
				if ((*m_gui->m_selected_object)->get_type() != e_node) {
					_info("selected object is not node");
				}
				else {
					c_node &node = unique_cast_ref<c_node &>(*m_gui->m_selected_object);
					node.send_osi3_data_to_dst(10020, std::string("example data")); // TODO !!! address
				}
			}
			
		//	selected_switch->send_hello_to_neighbors(); // TODO
			
            //shared_ptr<c_cjddev> selected_device = std::dynamic_pointer_cast<c_cjddev>(selected_object);
			if (selected_object != m_world->m_objects.end()) { // editing the selected object
				// TODO: add connect
				/*if (gui_mouse_b == 1 && !print_connect_line) {
					print_connect_line = true;
					connect_node = std::dynamic_pointer_cast<c_cjddev>(selected_object);
					last_click_time = std::chrono::steady_clock::now();
				}

				if (gui_mouse_b == 1 && print_connect_line &&
				    std::chrono::steady_clock::now() - last_click_time > std::chrono::milliseconds(500)) {
					// assert( connect_node && selected_object  );
					connect_node->add_neighbor(std::dynamic_pointer_cast<c_cjddev>(selected_object));
					(std::dynamic_pointer_cast<c_cjddev>(selected_object))->add_neighbor(std::dynamic_pointer_cast<c_cjddev>(connect_node));
					print_connect_line = false;
				}*/

				if (mode == e_mode_node) { // working with selected object - moving
					if (!print_connect_line) {
						int speed = 5;
						if (allegro_keys[KEY_LEFT])
							selected_object_raw->m_x += -speed;
						if (allegro_keys[KEY_RIGHT])
							selected_object_raw->m_x += speed;
						if (allegro_keys[KEY_DOWN])
							selected_object_raw->m_y += speed;
						if (allegro_keys[KEY_UP])
							selected_object_raw->m_y += -speed;
					}
				} // moving selected object
			}
            if ((allegro_char & 0xff) == 't' && selected_switch && !start_simulation) {
				_dbg1("badger T");

                for (auto &object : m_world->m_objects) {
                    object->m_target = false;
                }
                (*selected_object)->m_target = true;
                    m_gui->m_target_node = m_gui->m_selected_object;
					m_gui->m_target_ok = true; // mayby we should checking if m_gui->m_selected_object is switch?
			}

			if ((allegro_char & 0xff) == 'r' && selected_switch && !start_simulation) {
				_dbg1("badger R");
                for (auto &object : m_world->m_objects) {
                    object->m_source = false;
                }
                (*selected_object)->m_source = true;
                    m_gui->m_source_node = m_gui->m_selected_object;
					m_gui->m_source_ok = true; // mayby we should checking if m_gui->m_selected_object is switch?
			}

			// start simulation for switches
			if ((allegro_char & 0xff) == 's' && !start_simulation) {

				if (!m_gui->m_target_ok || !m_gui->m_source_ok) {
					_info("please choose target and source switch");
				} else {
					auto source = m_gui->m_source_node;
					auto target = m_gui->m_target_node;
					c_osi2_switch *source_node = dynamic_cast<c_osi2_switch*>((*source).get());
					c_osi2_switch *target_node = dynamic_cast<c_osi2_switch*>((*target).get());
					std::cout << "UUID SOURCE:" << source_node->get_uuid_any() << std::endl;
					std::cout << "UUID TARGET:" << target_node->get_uuid_any() << std::endl;
					t_osi3_packet pckg {"go go dijkstry",
										target_node->get_uuid_any(),
										source_node->get_uuid_any()};
					source_node->snd_pgk_test(std::move(pckg));
					start_simulation = true;
					simulation_pause = false;
				}
			}
/*
			if ((allegro_char & 0xff) == 's' && !start_simulation) {
				if (!m_gui->m_source || !m_gui->m_target) {
					std::cout << "please choose target and source node\n";
				} else {
					m_gui->m_source->buy_net(m_gui->m_target->get_address());
					start_simulation = true;
					simulation_pause = false;
				}
			}

			if ((allegro_char & 0xff) == 'd' && selected_device && !start_simulation) {
				for (shared_ptr<c_cjddev> &it : selected_device->get_neighbors()) {
					selected_device->remove_neighbor(it);
					it->remove_neighbor(selected_device);
				}

				for (auto it = m_world->m_objects.begin(); it != m_world->m_objects.end(); ++it) {
					if (it->get() == selected_device.get()) {
						m_world->m_objects.erase(it);
						break;
					}
				}
			}

			if ((allegro_char & 0xff) == 't' && selected_device && !start_simulation) {
				m_gui->m_target = selected_device;
			}

			if ((allegro_char & 0xff) == 'r' && selected_device && !start_simulation) {
				m_gui->m_source = selected_device;
			}

			if ((allegro_char & 0xff) == 'f') {
				if (!m_gui->m_source || !m_gui->m_target)
					std::cout << "please choose target and source node\n";
				else {
					m_gui->m_source->send_ftp_packet(m_gui->m_target->get_address(), "FTP data");
					last_click_time = std::chrono::steady_clock::now();
				}
			}
*/

			// === animation clock controll ===
			if ((allegro_char & 0xff) == 'p') {
				simulation_pause = !simulation_pause;
				last_click_time = std::chrono::steady_clock::now();
			}

			if (allegro_keys[KEY_MINUS_PAD] && g_max_anim_frame < 400 &&
			    std::chrono::steady_clock::now() - last_click_time > std::chrono::milliseconds(loop_miliseconds)) {
				//std::cout << m_frame_number-frame_checkpoint << " - " << g_max_anim_frame << " mod: " << (m_frame_number-frame_checkpoint) % g_max_anim_frame <<  std::endl;
				g_max_anim_frame += 1;
				last_click_time = std::chrono::steady_clock::now();
			}

			if (allegro_keys[KEY_PLUS_PAD] && g_max_anim_frame > 10 &&
			    std::chrono::steady_clock::now() - last_click_time > std::chrono::milliseconds(loop_miliseconds)) {
				//std::cout << m_frame_number-frame_checkpoint << " + " << g_max_anim_frame << " mod: " << (m_frame_number-frame_checkpoint) % g_max_anim_frame <<  std::endl;
				g_max_anim_frame -= 1;
				last_click_time = std::chrono::steady_clock::now();
			}
			
		}

		// === animation clock operations ===
		
        m_world->draw(*m_drawtarget.get()); // <===== DRAW THE WORLD

		/*
		if ((m_frame_number - frame_checkpoint) < g_max_anim_frame) {
			m_world->draw(*m_drawtarget.get(), (m_frame_number - frame_checkpoint) % g_max_anim_frame); // <==
		} else {
			m_world->draw(*m_drawtarget.get(), g_max_anim_frame);
		}
		*/

		if (print_connect_line) { // the line the creates new connections
			if (use_draw_allegro) {
				line(m_frame, connect_node->m_x, connect_node->m_y, allegro_mouse_x, allegro_mouse_y, makecol(0, 255, 255));
			}
            // TODO @opengl
            if (use_draw_opengl) {
                glColor3f(0.0f,1.0f,1.0f);
                glLineWidth(1.0);
                glScalef(1.0f,1.0f,1.0f);

                const int vx = m_gui->view_x(connect_node->m_x), vy = m_gui->view_y(connect_node->m_y); // position in viewport - because camera position
                //float start_line_x = ((connect_node->m_x)-0.5*SCREEN_W)/(0.5*SCREEN_W);
                //float start_line_y = -((connect_node->m_y)-0.5*SCREEN_H)/(0.5*SCREEN_H);
                float start_line_x = (vx-0.5*SCREEN_W)/(0.5*SCREEN_W);
                float start_line_y = -(vy-0.5*SCREEN_H)/(0.5*SCREEN_H);
                float end_line_x = (allegro_mouse_x-0.5*SCREEN_W)/(0.5*SCREEN_W);
                float end_line_y = -(allegro_mouse_y-0.5*SCREEN_H)/(0.5*SCREEN_H);

                //_dbg1("connect_node: " << connect_node->m_x << " " << connect_node->m_y);
                //_dbg1("allegro_mouse: " << allegro_mouse_x << " " << allegro_mouse_y);
                //_dbg1("start_line: " << start_line_x << " " << start_line_y);
                //_dbg1("end_line: " << end_line_x << " " << end_line_y);

                //glLoadIdentity();
                glPushMatrix();
                glBegin(GL_LINES);
                glVertex3f(start_line_x,start_line_y,0.0f);
                glVertex3f(end_line_x,end_line_y,0.0f);
                glEnd();
                glPopMatrix();
            }
		}
		if (allegro_mouse_b == 2) { // end/stop the line that creates new connections
			print_connect_line = false;
		}




		{
			auto x = allegro_mouse_x, y = allegro_mouse_y;
			int r = 5, rr = 4;

			if (use_draw_allegro) {
				line(m_frame, x - rr, y, x + rr, y, makecol(0, 0, 0));
				line(m_frame, x, y - rr, x, y + rr, makecol(0, 0, 0));
				circle(m_frame, x, y, r, makecol(255, 255, 255));
			}
			// TODO @opengl
            if(use_draw_opengl) {
                float opengl_mouse_x = (x-SCREEN_W*0.5)/(0.5*SCREEN_W);
                float opengl_mouse_y = -(y-SCREEN_H*0.5)/(0.5*SCREEN_H);
                float cursor_size=0.01;

                //_dbg1("mouse_x mouse_y: " << mouse_x << " " << mouse_y);
                //_dbg1("screenW screenH: " << SCREEN_W << " " << SCREEN_H);
                //glLoadIdentity();
                //glScalef(1/camera_offset, 1/camera_offset, 1.0);
                //glScalef(1.0*zoom, 1.0*zoom, 1.0*zoom);
                glPushMatrix();
                glScalef(1.0f,1.0f,1.0f);
                glTranslatef(opengl_mouse_x,opengl_mouse_y,0.0f);
                //glTranslatef(m_gui->view_x_rev(mouse_x),m_gui->view_y_rev(mouse_y),0.0f);
                glColor3f(0.0, 0.0, 0.0);

                // draw cursor
                glBegin(GL_LINES);
                glVertex2f(-1.0f*cursor_size,0.0f);
                glVertex2f(1.0f*cursor_size,0.0f);
                glVertex2f(0.0f,-1.0f*cursor_size);
                glVertex2f(0.0f,1.0f*cursor_size);
                glEnd();
                glPopMatrix();
            }

		}


		// === show frame ===

		if (use_draw_allegro) {
			//_dbg1("Allegro: frame done. fps = " << fps_str);
			scare_mouse();
			blit(m_frame, m_screen, 0, 0, 0, 0, m_frame->w, m_frame->h);
			unscare_mouse();
			if (!simulation_pause) {
				++m_frame_number;
			}
		}
		if (use_draw_opengl) {
            //_dbg1("OpenGL: frame flip. fps = " << fps_str);
			allegro_gl_flip();
		}

		for (auto &object : m_world->m_objects) {
			object->m_selected = false;
		}
		
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		auto stop_time = std::chrono::high_resolution_clock::now();
		auto diff = stop_time - start_time;
		loop_miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	}

	std::ofstream out_file("../layout/current/out.map.txt");
	out_file << *m_world << std::endl;
}




int c_simulation::get_move_object (int mouse_x, int mouse_y) {
	const int vx = m_gui->view_x_rev(mouse_x), vy = m_gui->view_y_rev(mouse_y); // position in viewport - because camera position

	double max_distance = 150;
	
	int move_index = -1;
	for (size_t i = 0; i < m_world->m_objects.size(); ++i) {
		int x1, x2, y1, y2;
		x1 = std::max(vx, m_world->m_objects.at(i)->get_x());
		x2 = std::min(vx, m_world->m_objects.at(i)->get_x());
		y1 = std::max(vy, m_world->m_objects.at(i)->get_y());
		y2 = std::min(vy, m_world->m_objects.at(i)->get_y());
		double current_dist = sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
		if (current_dist < max_distance) {
			max_distance = current_dist;
			move_index = i;
		}
	}
	return move_index;
}


volatile bool c_simulation::close_button_pressed = false;
