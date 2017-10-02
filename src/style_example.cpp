namespace n_style_example {

/*

namespace:              n_namespace
class:                  c_some_class
type:                   t_user_id // some simple type. Also, enum/enum class.
member type:            t_user_id
value in enum[class]:   e_enum_value

member_function:        member_function()
free function:          free_function()

member variable:        m_member_variable
member variable in t_*: member_variable // e.g.: my_point.x=5
local variable:         some_variable
function parameter:     some_parameter
function parameter:     _some_parameter // to avoid duplication when needed
function parameter:     arg_color or opt_color // sometimes to be more clear
static variable:        s_static_variable
global variable:        g_global_variable // use very rarerly!

template-argument:      template <typename T>  or T,U,V or T1,T2,T3 or TInt,TFloat



In std-like general classes (for stdplus and such) following changes:

namespace:              namespace
class:                  some_class
type:                   simple_class // some simple type
member type:            user_id_type // with _type at end
value in enum[class]:   enum_value

member_function:        member_function()
free function:          free_function()


*/

typedef long int t_textureid;
using t_user_id = long int;

struct t_point {
	int x,y;
};

struct t_ellipse {
	int x,y,rx,ry;
	bool ignore;
};

int check_and_fix(t_ellipse obj) {
	int err=0;
	bool moved=false;
	bool resized=false;
	// if styles:
	if (obj.ignore) return 0;
	if (obj.x<0 ) { obj.x=-obj.x; ++err; moved=true; }
	if (obj.y<0 ) { obj.y=-obj.y; ++err; moved=true; }
	if (obj.rx<1) { obj.rx=1;     ++err; resized=true; }
	if (obj.rx<1) { obj.rx=1;     ++err; resized=true; }
	if (moved && !resized) {
		// ...
		// ...
	}
	return err;
}

class c_human {
	public:
		c_human();
		virtual ~c_human();

	protected:
		int m_age; ///< years since birth
		static long int s_count_all; ///< count of all humans (now alive)
};
long int c_human::s_count_all; ///< count of all humans (now alive)


// ==================================================================

int foo(int b) {
	int a=42;
	if (a>b) {
		return a+b;
	}
	if (a<b) return a+b*2;
	return 0;
}

} // namespace


