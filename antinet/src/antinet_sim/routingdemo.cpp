#include "routingdemo.hpp"
#include "libs1.hpp"
#include <stdexcept>
#include <limits>
#include <set>
using std::set;
using std::min; using std::max;

const float EPSI=0.0001;

// #define _info(X) do { std::cerr<<X<<std::endl; } while(0)
#define check(X) do { if (!(X)) { std::cout<<"Failed check assert in " << __LINE__ << " " << __FILE__ << std::endl; \
	throw std::runtime_error("Failed check"); } } while(0)
#define BEGINEND(X) X.begin(),X.end()

template<typename T> using SPtr = std::shared_ptr<T>;
template<typename T> using WPtr = std::weak_ptr<T>;
template<typename T> bool WPtrCompare(const WPtr<T> & a, const WPtr<T> & b) { return &*a == &*b; }
template<typename T> SPtr<T> to_shared(const WPtr<T> &w) { return w.lock(); }
template<typename T> T NONZERO(T v) { check(v!=0); return v; }
template<typename T, typename N> T subrange(const T &v, N start, N len) { if (len==0) return v;
	auto s=v.size(); if (!s) return T();
	auto pos1=min(s-1,static_cast<decltype(s)>(start)),	pos2=min(s-1,pos1+len); 	return T(v.begin()+pos1,v.begin()+pos2);
}
template<typename T, typename T2> T clamp(T v, T2 vmin, T2 vmax) {	if (v<vmin) return static_cast<T>(vmin);
	if (v>vmax) return static_cast<T>(vmax); return v; }

bool rand1in(int n) { check(n>=1); check(n<RAND_MAX); return 0 == (rand()%n); }
float rand01() { return rand() % RAND_MAX / float(RAND_MAX); }
float randm11() { return rand01()*2 - 1; }
float rand1pm(float d) { return 1 + rand01()*d; }
float randnorm(float mean, float dev) {	static std::default_random_engine rnd;
	std::normal_distribution<double> distr(mean,dev);	return distr(rnd); }

int toint(float f) { return static_cast<int>(f); }
long col(int r,int g, int b) { return makecol(r,g,b); }
long colf(float r,float g, float b) { return makecol(r*255,g*255,b*255); }

struct P { float x, y; P(float x_, float y_) : x(x_), y(y_) {}
	float dist(P p) const { return sqrt( pow(x-p.x,2) + pow(y-p.y,2) ); }
	P operator-(const P&other) { return {x-other.x, y-other.y}; }
	P operator+(const P&other) { return {x+other.x, y+other.y}; }
	P operator-() const { return {-x,-y}; }
	P& operator+=(const P&other) { x+=other.x; y+=other.y;  return *this; }
	P& operator-=(const P&other) { x-=other.x; y-=other.y;  return *this; }
	P operator*(float f) { return {x*f,y*f}; }
	P norm() const { auto l=dist({0,0}); if (fabs(l)<EPSI) return {0,0}; return {x/l,y/l}; }
};
typedef enum  { e_red, e_mark } t_flag;
struct NODE {
	P p,move; int lev;
	vector<WPtr<NODE>> peer;  set<t_flag> flag;
	NODE(P p_, int lev_):p(p_),move({0,0}),lev(lev_){}
	void flagadd(t_flag f_) { flag.insert(f_); }	void flagdel(t_flag f_) { flag.erase(f_); } void flagclr() { flag.clear(); }
	bool flagis(vector<t_flag> ftab) { for(auto o:ftab) { if(flag.count(o)) return true; } return false; }
	bool linkis(WPtr<NODE> that) { for(const auto &ptr:peer) { if (&*ptr.lock() == &*that.lock()) return true; } return false; }
	bool link(WPtr<NODE> that) { if ((&*that.lock()==&*this) || linkis(that)) return false; peer.push_back(that); return true; }
	void draw(BITMAP *frame) {
		circle(frame,p.x,p.y,5, flagis({e_red,e_mark}) ? colf(1,0,0) : colf(1,1,1) );
		if (flagis({e_mark})) circlefill(frame,p.x,p.y,5, colf(1,.3,0));
		for(auto pw: peer) { auto ps=pw.lock(); auto&o=*ps; line(frame,p.x,p.y,o.p.x,o.p.y,makecol(100,100,100)); }
	}
	void jumpaway() { move+=P(randm11(),randm11())*25; }
};
struct WORLD { vector<SPtr<NODE>> nodes;
	SPtr<NODE> grow(P p, int lev, int r) {
		SPtr<NODE> main = add(p,lev);
		auto nextlev = lev+1; int maxlev=4 + rand1in(60) + rand1in(800); bool is_last = nextlev >= maxlev;
		int cc=toint( (is_last ? 8 : 5) * (1+0.5*randm11())); // c-child. cc-child count, ci-index, cd-distance radius
		auto a0=rand01()*M_PI*2;
		for (int ci=0; ci<cc; ++ci) {
			float a1 = a0 + rand1pm(0.2)*(ci/float(cc+1))*M_PI*2, cd=r*(1 + 0.3*randm11());
			P nextp(p.x+sin(a1)*cd, p.y+cos(a1)*cd); auto nextr=r*0.3*randnorm(1,0.4); // r*0.3*(1+0.1*randm11());
			if (is_last) { auto that=add(nextp, lev+1); main->link(that); }
			else { auto that=this->grow(nextp, nextlev, nextr); main->link(that); }
		}
		return main;
	}
	SPtr<NODE> add(P p, int lev) { auto n=make_shared<NODE>(p,lev); nodes.push_back(n); return n;	}
	SPtr<NODE> node_any() { return nodes.at(rand()%NONZERO(nodes.size())); }
	vector<SPtr<NODE>> nodes_in_range(const NODE &n, float r, int count) {
		_info("r="<<r<<" count="<<count);
		decltype(nodes) near; for(auto &ptr:nodes) if (n.p.dist(ptr->p)<r) { near.push_back(ptr); }
		std::random_shuffle(BEGINEND(near));	return subrange(near,0,count);
	}
	vector<SPtr<NODE>> nodes_close(const NODE &n, float rtimes, int count) { return nodes_in_range(n, measure_close(n) * rtimes, count); }
	float measure_close(const NODE &n) { auto f=std::numeric_limits<float>::max();
		for (auto &ptr:nodes)	{ if(&n == &*ptr)continue;  auto d=ptr->p.dist( n.p );  if (d<f) f=d; } return f;	}
};

WORLD * world=nullptr;

bool c_simulation::routingdemo() {
	while (true) {
		bool ok = routingdemo_main();
		if (!ok) break;
	}
	return true;
}

bool c_simulation::routingdemo_main() {
	g_dbg_level_set(10,"Routing demo");
	bool do_it_again=false;
	auto & frame = m_frame; m_goodbye=false;

	world = new WORLD;
	auto world_root = world->grow(P{600,500},1,300);
	world->grow(P{1200,500},1,300);
	world_root->flag.insert(e_red);

	while (!m_goodbye && !close_button_pressed) {	try {
		process_input(); auto &xkey=m_gui->m_key; // int allegro_char = 0;	if (keypressed()) { allegro_char = readkey(); }
		clear_to_color (m_frame, makecol (32, 32, 32));
		if (m_gui->m_key[KEY_ESC]) { _note ("User exits)"); m_goodbye = true; }
		if (m_gui->m_key[KEY_ENTER]) { _note ("User exits - but do it again)"); m_goodbye = true; do_it_again=true; rest(10); }

		for (int step=0; step<5; ++step) {
		for(auto obj1 : world->nodes) obj1->move=P({0,0});
		if (xkey[KEY_1]) for(auto obj1 : world->nodes) { for(auto obj2 : world->nodes) { if (obj1==obj2) continue; // avoid nodes collision
			auto p1=obj1->p, p2=obj2->p; auto d=obj1->p.dist(obj2->p);  if (fabs(d)<7) { obj1->jumpaway(); continue; }
			if (d>30) continue; auto s=10/clamp(d,5,30); obj1->move += (p1-p2).norm() * pow(s,2);
		} }
		if (xkey[KEY_2]) for(auto obj1 : world->nodes) { for(auto obj2w : obj1->peer) { auto obj2=obj2w.lock(); check( obj1 != obj2); // pull peers
			auto p1=obj1->p, p2=obj2->p; auto d=obj1->p.dist(obj2->p);  if (fabs(d)<7) { obj1->jumpaway(); continue; }
			if (d>100) obj1->move += - (p1-p2).norm() * 3;
		} }
		for(auto obj1 : world->nodes) obj1->p += obj1->move;

		if (xkey[KEY_3]) if (rand1in(5)) {	int ntry=0; while (++ntry<100) { auto nnn = world->node_any(); if(nnn->peer.size()>2) continue;
			for(auto const &ptr : world->nodes_close(*nnn, randnorm(5.0,1.0), clamp(randnorm(2,1),0,5))) {
				if(ptr->peer.size()<2) { ptr->flagadd({e_red}); nnn->link(ptr); }
			}
			nnn->flagadd({e_mark});
			break;
			}
		}

		} // step

		for(auto obj1 : world->nodes) obj1->draw(frame);
		scare_mouse();  blit (m_frame, m_screen, 0, 0, 0, 0, m_frame->w, m_frame->h);  unscare_mouse();	rest(1);
	} catch(...) { _erro("Main loop - exception"); throw;	} }

	delete world; world=nullptr;
	return do_it_again;
}

