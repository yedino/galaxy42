#include "libs1.hpp"

namespace test3 {

/*

c_some_class { };
t_simple_type; // some very simple type, e.g. a POD
m_member_variable;
s_static_variable;
g_global_variable; // use very rarerly!
e_enum_value;

member_function();
regular_function();
some_variable;

*/

typedef long int t_textureid;
struct t_point { int x,y; };

class c_human {
	public:
		c_human();
		virtual ~c_human();
		
		void say_something(); ///< do some stuff
		virtual void think()=0; ///< process informations
		virtual void think_about_friend(const c_human & other); ///< think about your friend to decide your goals for today
		virtual void work_for_someone(const c_human & boss);
	protected:
		int age; ///< years since birth
		
		static long int s_count_all; ///< count of all humans (now alive)
};

/**
 * An enum of common currencies
 */
typedef enum { 
	e_currency_USD,
	e_currency_BTC,
	e_currency_PLN,
	e_currency_A,
	e_currency_B,
	e_currency_C
} t_currency;

class c_human_normal : public c_human {
	public:
		virtual void think();
	protected:
		string m_rigcz; ///< the "RiGCZ" data (see README)
		virtual void work_for_someone(const c_human & boss);
};

class c_socialist : public c_human {
	public:
		c_socialist();
		list<string> m_demands; ///< what we demand. In format of english text, one demand per entry.
		virtual void work_for_someone(const c_human & boss);
		virtual void think();
	protected:
		mutable double m_white_guilt; ///< mutable, can be modified by functions that are usually constant
};

extern c_human *g_you; ///< global (extern) variable with yourself

// ==================================================================

int foo();

// ==================================================================
// ==================================================================
/// --- this goes in .cpp file, but placed here for easier reading --- 
// ==================================================================
// ==================================================================

// #include <style.hpp>

// shortcuts for this implementation
using std::cout; 
using std::cerr; 
using std::endl; 

c_human *g_you = nullptr; // (extern)

c_human::c_human() : age(20) 
{
	_info("A human was created");
}

c_human::~c_human() 
{ }

void c_human::say_something() {
	cout << "test";
	if (age>100) {
		cout << "congratulation "
		     << "on your age of "
		     << age 
		     << endl;
		if (0==(age%10)) cout << "Another decade, good job" << endl;
	}
}

void c_human::think_about_friend(const c_human &other) { }
void c_human::work_for_someone(const c_human &boss) { }

/*
void c_human::think_about_friend(const c_human &other) {
}
void c_human::work_for_someone(const c_human &boss) {
}
*/

// ==================================================================

void c_human_normal::think() {
	// (stub function for now)
}

void c_human_normal::work_for_someone(const c_human &boss) {
	think();
}

// ==================================================================

c_socialist::c_socialist() : m_white_guilt(90) 
{ 
	m_demands.push_back("something");
}

void c_socialist::think() {
}

void c_socialist::work_for_someone(const c_human & boss) {
	m_white_guilt++;
	try {
		auto boss_soc = dynamic_cast<const c_socialist & > ( boss );
		boss_soc.m_white_guilt += 10; // ok, this is mutable field
	}
	catch(...) { }
}

// ==================================================================

int foo(int b) {
	int a=42;
	if (a>b) {
		return a+b;
	}
	return 0;
}

}

int main() { }

