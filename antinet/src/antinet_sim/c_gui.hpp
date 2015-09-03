#ifndef C_GUI_HPP
#define C_GUI_HPP

#include "libs1.hpp"
#include "c_gui.hpp"

#include "c_object.hpp"

/***
The state of user interface
*/
class c_gui {
public:
	weak_ptr<c_entity> m_selected_object; // current object
	shared_ptr<c_cjddev> m_target, m_source;
};

#endif // include guard

