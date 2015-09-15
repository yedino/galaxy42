#include "c_msg.hpp"


msgcjd::msgcjd (const t_msgkind &logic) : m_logic(logic), m_ID(std::rand()) // TODO
{
}

msg_buy::msg_buy (const t_msgkind &logic) : msgcjd(logic) {
}

msg_buy_inq::msg_buy_inq () : msg_buy(e_msgkind_buy_net_inq) {
}

msg_buy_menu::msg_buy_menu () : msg_buy(e_msgkind_buy_net_menu) {
}

msg_buy_buying::msg_buy_buying () : msg_buy(e_msgkind_buy_net_buying) {
}

msg_buy_agreed::msg_buy_agreed () : msg_buy(e_msgkind_buy_net_agreed) {
}

msg_buy_final::msg_buy_final () : msg_buy(e_msgkind_buy_net_final) {
}

msg_buy_currency_inq::msg_buy_currency_inq() : msg_buy(e_msgkind_buy_currency) {
}

msg_use::msg_use () : msgcjd(e_msgkind_data) {
}

msg_use_ftp::msg_use_ftp () {
}
