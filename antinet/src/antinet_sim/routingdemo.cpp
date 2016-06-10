#include "routingdemo.hpp"
#include "libs1.hpp"
#include <stdexcept>
#include <limits>

#define check(X) do { if (!(X)) { std::cout<<"Failed check assert in " << __LINE__ << " " << __FILE__ << std::endl; \
	throw std::runtime_error("Failed check"); } } while(0)

template<typename T> using SPtr = std::shared_ptr<T>;
template<typename T> using WPtr = std::weak_ptr<T>;
template<typename T> SPtr<T> to_shared(const WPtr<T> &w) { return w.lock(); }
template<typename T> T NONZERO(T v) { check(v!=0); return v; }

float rand01() { return rand() % RAND_MAX / float(RAND_MAX); }
float randm11() { return rand01()*2 - 1; }
float rand1pm(float d) { return 1 + rand01()*d; }
int toint(float f) { return static_cast<int>(f); }

struct P { int x, y; P(float x_, float y_) : x(x_), y(y_) {}
	float dist(P p) const { return sqrt( pow(x-p.x,2) + pow(y-p.y,2) ); }
};
struct NODE {
	P p; int lev;
	vector<WPtr<NODE>> peer;
	NODE(P p_, int lev_):p(p_),lev(lev_){}
	void link(WPtr<NODE> that) { peer.push_back(that); }
	void draw(BITMAP *frame) {
		circle(frame,p.x,p.y,5,makecol(255,255,255));
		for(auto pw: peer) { auto ps=pw.lock(); auto&o=*ps; line(frame,p.x,p.y,o.p.x,o.p.y,makecol(100,100,100)); }
	}
};
struct WORLD { vector<SPtr<NODE>> nodes;
	WPtr<NODE> grow(P p, int lev, int r) {
		SPtr<NODE> main = to_shared(add(p,lev));
		auto nextlev = lev+1; bool is_last = nextlev>=4;
		int cc=toint( (is_last ? 8 : 5) * (1+0.5*randm11())); // c-child. cc-child count, ci-index, cd-distance radius
		auto a0=rand01()*M_PI*2;
		for (int ci=0; ci<cc; ++ci) {
			float a1 = a0 + rand1pm(0.2)*(ci/float(cc+1))*M_PI*2, cd=r*(1 + 0.3*randm11());
			P nextp(p.x+sin(a1)*cd, p.y+cos(a1)*cd); auto nextr=r*0.3*(1+0.1*randm11());
			if (is_last) { auto that=add(nextp, lev+1); main->link(that); }
			else { auto that=this->grow(nextp, nextlev, nextr); main->link(that); }
		}
		return main;
	}
	WPtr<NODE> add(P p, int lev) { auto n=make_shared<NODE>(p,lev); nodes.push_back(n); return n;	}
	WPtr<NODE> node_any() { return nodes.at(rand()%NONZERO(nodes.size())); }
	WPtr<NODE> node_in_range(const NODE &n, float r) {
		decltype(nodes) near; for(auto &ptr:nodes) if (n.p.dist(ptr->p)<r) { near.push_back(ptr); }
		return near.at(rand() % NONZERO(near.size()));
	}
	WPtr<NODE> node_close(const NODE &n, float rtimes) { return node_in_range(n, measure_close(n) * rtimes); }
	float measure_close(const NODE &n) { auto f=std::numeric_limits<float>::max();
		for (auto &ptr:nodes) { auto d=ptr->p.dist( n.p ); if (d<f) f=d; } return f;
	}

};

WORLD * world=nullptr;

void c_simulation::routingdemo() {
	world = new WORLD;
	world->grow(P{600,500},1,300);
	auto & frame = m_frame;
	while (!m_goodbye && !close_button_pressed) {	try {
		process_input();  // int allegro_char = 0;	if (keypressed()) { allegro_char = readkey(); }
		clear_to_color (m_frame, makecol (32, 32, 32));
		if (m_gui->m_key[KEY_ESC]) { _note ("User exits the simulation from user interface"); m_goodbye = true; }

		for(auto obj : world->nodes) obj->draw(frame);

		scare_mouse();  blit (m_frame, m_screen, 0, 0, 0, 0, m_frame->w, m_frame->h);  unscare_mouse();
	} catch(...) { _erro("Main loop - exception"); throw;	} }

	delete world; world=nullptr;
}

