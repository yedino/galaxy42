#include "c_simulation.hpp"
#include "c_gui.hpp"

unsigned int g_max_anim_frame = 10;

c_simulation::c_simulation () : m_goodbye(false), m_frame_number(0), m_frame(nullptr), m_screen(screen) { }

c_simulation::~c_simulation () {
	if (m_frame)
		destroy_bitmap(m_frame);
	m_frame = nullptr;
}

void c_simulation::init () {
	//this->close_button_pressed = false;
	m_screen = screen;
	m_frame = create_bitmap(m_screen->w, m_screen->h);
	m_world = make_unique<c_world>();
	m_world->add_test();
	//m_world->add_i_objects(100);
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

	while (!m_goodbye && !close_button_pressed) {
		std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
		// process the keyboard/GUI
		poll_keyboard();
		auto allegro_keys = key;
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
		if (keypressed())
			allegro_char = readkey();

		if ((allegro_char & 0xff) == 'n' && !start_simulation) {
			std::cout << "ADD" << std::endl;
			m_world->m_objects.push_back(make_shared<c_cjddev>(cjddev_detail_random_name(), gui_mouse_x, gui_mouse_y, cjddev_detail_random_addr()));
		}

		if (allegro_keys[KEY_ESC] || allegro_keys[KEY_ENTER]) {
			m_goodbye = true;
		}

		clear_to_color(m_frame, makecol(0, 128, 0));
		blit(c_bitmaps::get_instance().m_background, m_frame, 0, 0, viewport_x, viewport_y, c_bitmaps::get_instance().m_background->w, c_bitmaps::get_instance().m_background->h);

		if (m_frame_number % g_max_anim_frame == 0 && !simulation_pause) {
			frame_checkpoint = m_frame_number;
			m_world->tick(); // <===
		}

		string mouse_pos_str = std::to_string(gui_mouse_x) + " " + std::to_string(gui_mouse_y);
		string fps = "fps ???";
		if (loop_miliseconds != 0) {
			fps = "fps: " + std::to_string(1000 / loop_miliseconds);
		}

		const int txt_h = 12; // line height (separation between lines)
		int txt_x = 10, txt_y = 10; // starting position of text

		string pck_speed_str = "sending packets speed - " + std::to_string(450 - g_max_anim_frame);
		textout_ex(m_frame, font, mouse_pos_str.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, fps.c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, ("Frame nr.: " +
		                           std::to_string(m_frame_number)).c_str(), txt_x, txt_y += txt_h, makecol(0, 0, 255), -1);

		textout_ex(m_frame, font, pck_speed_str.c_str(), 100, 10, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "s - start", 1140, 5, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "p - pause", 1140, 15, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "f - send FTP", 1140, 25, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "t - select target", 1140, 35, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "r - select source", 1140, 45, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "d - remove node", 1140, 55, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "n - add node", 1140, 65, makecol(0, 0, 255), -1);
		textout_ex(m_frame, font, "enter/esc - exit", 1140, 75, makecol(0, 0, 255), -1);


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

		{
			scare_mouse();
			blit(m_frame, m_screen, 0, 0, 0, 0, m_frame->w, m_frame->h);
			unscare_mouse();
			if (!simulation_pause) {
				++m_frame_number;
			}
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
	double max_distance = 150;
	shared_ptr<c_entity> ret_ptr;
	for (auto node : m_world->m_objects) {
		shared_ptr<c_entity> node_ptr = std::dynamic_pointer_cast<c_entity>(node);
		int x1, x2, y1, y2;
		x1 = std::max(mouse_x, node_ptr->m_x);
		x2 = std::min(mouse_x, node_ptr->m_x);
		y1 = std::max(mouse_y, node_ptr->m_y);
		y2 = std::min(mouse_y, node_ptr->m_y);
		double current_dist = sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
		if (current_dist < max_distance) {
			max_distance = current_dist;
			ret_ptr = node_ptr;
		}
	}
	return ret_ptr;
}


volatile bool c_simulation::close_button_pressed = false;
