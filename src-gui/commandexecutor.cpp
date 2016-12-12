#include "commandexecutor.hpp"


std::shared_ptr<commandExecutor> commandExecutor::construct(std::shared_ptr<MainWindow> window) {
	std::shared_ptr<commandExecutor>ret = std::make_shared<commandExecutor>(window);
	ret->m_net_client = std::make_shared<netClient>(ret);
	return ret;
}

void commandExecutor::parseAndExecMsg(const std::string &msg) {
	order input_order(msg);
	const std::string cmd = input_order.get_cmd();
	std::cout << "execute cmd: " << cmd << "\n";
	auto main_window_ptr = m_main_window.lock();
	main_window_ptr->add_to_debug_window("get message " + msg);
	if (cmd == "ping") {
		// TODO show on main window
	}
	else if (cmd == "peer_list") {
		main_window_ptr->show_peers(input_order.get_msg_array());
	}
}

void commandExecutor::sendNetRequest(const order &ord) {
	m_net_client->send_msg(ord.get_str());
}

void commandExecutor::startConnect(const QHostAddress &address, uint16_t port) {
	m_net_client->startConnect(address, port);
}

order::order(const std::string &json_str) {
	using nlohmann::json;
	json j = json::parse(json_str);
	m_cmd = j["cmd"];
	if (m_cmd == "peer_list") {
		//m_msg_array = j["msg"];
		std::vector<std::string> tmp = j["msg"];
		m_msg_array = std::move(tmp);
	} else {
		m_msg = j["msg"];
	}
}

order::order(order::e_type cmd) {
	if (cmd == e_type::PING) {
		m_cmd = "ping";
		m_msg = "ping";
	}
	else if (cmd == e_type::PEER_LIST) {
		m_cmd = "peer_list";
	}
}

std::string order::get_str() const {
	nlohmann::json j{{"cmd", m_cmd} , {"msg", m_msg}};
	return j.dump();
}

std::string order::get_cmd() const {
	return m_cmd;
}

std::string order::get_msg() const {
	return m_msg;
}

std::vector<std::string> order::get_msg_array() const {
	return m_msg_array;
}

commandExecutor::commandExecutor(std::shared_ptr<MainWindow> window)
:
	m_main_window(window),
	m_net_client(nullptr),
	m_timer(std::make_unique<QTimer>())
{
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(timer_slot()));
	m_timer->start(5000);
}

void commandExecutor::timer_slot() {
	order peer_list_order(order::e_type::PEER_LIST);
	m_net_client->send_msg(peer_list_order.get_str());
	qDebug() << "timer slot";
}
