#include "c_simulation.hpp"
#include "c_gui.hpp"
#include <stdlib.h>
#include "use_opengl.hpp"

unsigned int g_max_anim_frame = 10;

c_simulation::c_simulation (t_drawtarget_type drawtarget_type) 
: 
	m_goodbye(false), m_frame_number(0), 
	m_drawtarget_type( drawtarget_type ),
	m_frame(nullptr), 
	m_screen( m_drawtarget_type == e_drawtarget_type_allegro ? screen : nullptr),
	m_drawtarget(nullptr)
{
	_note("Creating simulation, mode: " << t_drawtarget_type_to_string( m_drawtarget_type ) << ". The allegro screen is at " << (void*)m_screen );
}

c_simulation::~c_simulation () {
	if (m_frame)
		destroy_bitmap(m_frame);
    m_frame = nullptr;
    if(smallWindow){
        destroy_bitmap (smallWindow);
    }
    smallWindow = nullptr;
}

void c_simulation::init () {
	//this->close_button_pressed = false;
	_note("We are using m_screen=" << m_screen);

	if (m_drawtarget_type == e_drawtarget_type_allegro) {
		_note("Creating the allegro frame buffer for m_screen="<<m_screen);
		_note("Screen size is: " << m_screen->w << "*" << m_screen->h );
		m_frame = create_bitmap(m_screen->w, m_screen->h);
		_note("Done, the m_frame="<<m_frame);
	}
	
	smallWindow = create_bitmap(m_screen->w,m_screen->h); 

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

int cjddev_detail_random_addr () {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
	return distribution(generator);
}


void c_simulation::main_loop () {
	//	PALETTE palette;
	//BITMAP *img_bgr = load_bitmap("dat/bgr-bright.tga", NULL); // TODO:

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
	m_drawtarget = make_shared<c_drawtarget_allegro>(m_frame); // prepare drawtarget surface to draw to
	m_drawtarget->m_gui = m_gui;



	//	bool allegro_keys_any_was=false; // is any key pressed right now (for key press/release)
	long loop_miliseconds = 0;
	long unsigned int frame_checkpoint = 0; /// needed for speed control (without world_draw manipulate in spacetime!)

	bool use_input_allegro = true; // always for now.  input from Allegro
	bool use_draw_allegro = m_drawtarget_type == e_drawtarget_type_allegro; // draw in allegro
	bool use_draw_opengl = m_drawtarget_type == e_drawtarget_type_opengl; // draw in opengl


	_note("Entering main simulation loop");

	// === main loop ===
	while (!m_goodbye && !close_button_pressed) { 
		std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

		// --- process the keyboard/inputs ---
		if (use_input_allegro) {
				// TODO move this code here, but leave the variables in higher scope
		}

			poll_keyboard();
			auto allegro_keys = key;
			auto allegro_shifts = key_shifts;
			//		bool allegro_keys_any_is=false;
			//		for (size_t i=0; i<sizeof(allegro_keys)/sizeof(allegro_keys[0]); ++i) allegro_keys_any_is=true;
			int allegro_mouse_x, allegro_mouse_y, allegro_mouse_b;
			allegro_mouse_x = mouse_x;
			allegro_mouse_y = mouse_y;
			allegro_mouse_b = mouse_b;

			int gui_mouse_x = allegro_mouse_x;
			int gui_mouse_y = allegro_mouse_y;
			int gui_mouse_b = allegro_mouse_b;

			int allegro_char = 0;
			if (keypressed()) {
				allegro_char = readkey();
			}
		// end of input
		

		// background frame
		if (use_draw_allegro) {
			clear_to_color(m_frame, makecol(0, 128, 0));
			blit(c_bitmaps::get_instance().m_background, m_frame, 0, 0, viewport_x, viewport_y, c_bitmaps::get_instance().m_background->w, c_bitmaps::get_instance().m_background->h);
		}
		if  (use_draw_opengl) {
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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
			std::cout << "ADD" << std::endl;
			m_world->m_objects.push_back(make_shared<c_cjddev>(cjddev_detail_random_name(), gui_mouse_x, gui_mouse_y, cjddev_detail_random_addr()));
		}


        if(allegro_keys[KEY_F1]){
            auto ptr = get_move_object(gui_mouse_x,gui_mouse_y);
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
            }catch(...)
            {}
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



		// === text debug on screen ===

		string mouse_pos_str = std::to_string(gui_mouse_x) + " " + std::to_string(gui_mouse_y);
		string fps = "fps ???";
		if (loop_miliseconds != 0) {
			fps = "fps: " + std::to_string(1000 / loop_miliseconds);
		}

		const int txt_h = 12; // line height (separation between lines)
		int txt_x = 10, txt_y = 10; // starting position of text

		if (use_draw_allegro) {

			string pck_speed_str = "sending packets speed - " + std::to_string(450 - g_max_anim_frame);
			textout_ex(m_frame, font, mouse_pos_str.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
			textout_ex(m_frame, font, fps.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
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
		}


		// find out which object is selected:
		m_gui->m_selected_object = get_move_object(gui_mouse_x, gui_mouse_y);

		{ // working with selected object
			shared_ptr<c_entity> selected_object = m_gui->m_selected_object.lock();
			shared_ptr<c_cjddev> selected_device = std::dynamic_pointer_cast<c_cjddev>(selected_object);
			if (selected_object) { // editing the selected object
				if (gui_mouse_b == 1 && !print_connect_line) {
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
				}

				if (mode == e_mode_node) { // working with selected object - moving
					if (!print_connect_line) {
						int speed = 5;
						if (allegro_keys[KEY_LEFT])
							selected_object->m_x += -speed;
						if (allegro_keys[KEY_RIGHT])
							selected_object->m_x += speed;
						if (allegro_keys[KEY_DOWN])
							selected_object->m_y += speed;
						if (allegro_keys[KEY_UP])
							selected_object->m_y += -speed;
					}
				} // moving selected object
			}

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

		if ((m_frame_number - frame_checkpoint) < g_max_anim_frame) {
			m_world->draw(*m_drawtarget.get(), (m_frame_number - frame_checkpoint) % g_max_anim_frame); // <==
		} else {
			m_world->draw(*m_drawtarget.get(), g_max_anim_frame);
		}
		if (print_connect_line) { // the line the creates new connections
			line(m_frame, connect_node->m_x, connect_node->m_y, allegro_mouse_x, allegro_mouse_y, makecol(0, 255, 255));
		}
		if (allegro_mouse_b == 2) { // end/stop the line that creates new connections
			print_connect_line = false;
		}
		{
			auto x = allegro_mouse_x, y = allegro_mouse_y;
			int r = 5, rr = 4;
			line(m_frame, x - rr, y, x + rr, y, makecol(0, 0, 0));
			line(m_frame, x, y - rr, x, y + rr, makecol(0, 0, 0));
			circle(m_frame, x, y, r, makecol(255, 255, 255));
		}


		// === show frame ===

		if (use_draw_allegro) {
			scare_mouse();
			blit(m_frame, m_screen, 0, 0, 0, 0, m_frame->w, m_frame->h);
			unscare_mouse();
			if (!simulation_pause) {
				++m_frame_number;
			}
		}
		if (use_draw_opengl) {
			 allegro_gl_flip();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::chrono::time_point<std::chrono::steady_clock> stop_time = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration diff = stop_time - start_time;
		loop_miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	}

	std::ofstream out_file("../layout/current/out.map.txt");
	out_file << *m_world << std::endl;
}





shared_ptr<c_entity> c_simulation::get_move_object (int mouse_x, int mouse_y) {
	const int vx = m_gui->view_x_rev(mouse_x), vy = m_gui->view_y_rev(mouse_y); // position in viewport - because camera position

    double max_distance = 150;
	shared_ptr<c_entity> ret_ptr;
	for (auto node : m_world->m_objects) {
		shared_ptr<c_entity> node_ptr = std::dynamic_pointer_cast<c_entity>(node);
		int x1, x2, y1, y2;
		x1 = std::max(vx, node_ptr->m_x);
		x2 = std::min(vx, node_ptr->m_x);
		y1 = std::max(vy, node_ptr->m_y);
		y2 = std::min(vy, node_ptr->m_y);
		double current_dist = sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
		if (current_dist < max_distance) {
			max_distance = current_dist;
            ret_ptr = node_ptr;
        }
	}
	return ret_ptr;
}


volatile bool c_simulation::close_button_pressed = false;
