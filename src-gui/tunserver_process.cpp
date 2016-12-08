#include "tunserver_process.hpp"
#include <regex>
#include <boost/asio.hpp>

#include <QDebug>
#include <QFileDialog>

std::string peer_reference::to_string() const {
	std::string str = m_ipv4 + ":";
	str += std::to_string(m_port);
	str += "-" + m_ipv6;
	return str;
}

peer_reference peer_reference::get_validated_ref(std::string ref) {
	std::string r_ipv4_port;
	std::string r_ipv4;
	std::string r_port;
	std::string r_ipv6;

	size_t pos1;
	if ((pos1 = ref.find('-')) != std::string::npos) {
		r_ipv4_port = ref.substr(0,pos1);
		r_ipv6 = ref.substr(pos1+1);

		std::regex pattern("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}"); // i.e. 127.0.0.1:4562
		std::smatch result;
		qDebug()<<r_ipv4_port.c_str();
		if (std::regex_search(r_ipv4_port, result, pattern)) {
			size_t pos2 = r_ipv4_port.find(':');
			r_ipv4 = r_ipv4_port.substr(0, pos2);
			r_port = r_ipv4_port.substr(pos2+1);
		} else
			throw std::invalid_argument("bad format of input remote address and port");

		boost::system::error_code ec;
		// validate ipv4
		boost::asio::ip::address_v4::from_string(r_ipv4, ec);
		if (ec)
			throw std::invalid_argument("bad format of input remote IPv4 address");
		// validate ipv6
		boost::asio::ip::address_v6::from_string(r_ipv6, ec);
		if(ec)
			throw std::invalid_argument("bad format of input remote IPv6 address");

	} else
		throw std::invalid_argument("bad format of input ref - missing '-'");
	return {r_ipv4 ,stoi(r_port), r_ipv6};
}

tunserverProcess::tunserverProcess() :
	m_tunserver_process(nullptr)
{ }

tunserverProcess::~tunserverProcess() {
	if(is_alive())
		m_tunserver_process->kill();
}

void tunserverProcess::run() {
	if(is_alive())
		m_tunserver_process->kill();

	QString tunserver_path = get_path();
	QStringList params_list = prepare_params();

	try {
		m_tunserver_process = std::make_unique<QProcess>(this);
		m_tunserver_process->start(tunserver_path , params_list);

		if(m_tunserver_process->state() == QProcess::NotRunning)
			throw std::runtime_error("Fail to run tunserver process");
	} catch (const std::exception &err) {
		qDebug() << err.what();
	}

}

void tunserverProcess::kill() {
	m_tunserver_process->kill();
	m_tunserver_process->reset();
}

std::vector<peer_reference> tunserverProcess::get_peer_list() const {
	return m_peer_lst;
}

void tunserverProcess::add_peer(const peer_reference &ref) {
	m_peer_lst.push_back(ref);
}

void tunserverProcess::del_peer(const peer_reference &ref) {

	const auto &it = std::find_if_not(m_peer_lst.begin(), m_peer_lst.end(),
		[&ref](const auto &a){
				return ref.m_ipv6 == a.m_ipv6;
		}
	);
	if(it != m_peer_lst.end())
		m_peer_lst.erase(it);

}

void tunserverProcess::onProcessInfo() {
	qDebug()<<m_tunserver_process->readAll();
}

void tunserverProcess::onProcessError() {
	qDebug()<<m_tunserver_process->readAll();
}

void tunserverProcess::add_address(QString address)
{
	qDebug()<< "add address [" << address << ']';
	try {
		peer_reference peer_ref = peer_reference::get_validated_ref(address.toStdString());
		m_peer_lst.push_back(peer_ref);

	} catch (std::exception &er) {
		qDebug()<< er.what();
	} catch (...) {
		qDebug()<< "fail to parse address - bad format";
	}

}

QString tunserverProcess::get_path() {
	return QFileDialog::getOpenFileName(0, tr("Open tunserver binary file (tunserver.elf)"));
}

QStringList tunserverProcess::prepare_params() {

	QStringList params_list;
	for (const auto &peer : m_peer_lst) {
		std::string peer_string = " --peer ";
		peer_string += peer.m_ipv4 + ":" + std::to_string(peer.m_port);
		peer_string += "-" + peer.m_ipv6;
		params_list.push_back(QString::fromStdString(peer_string));
	}

	qDebug()<<"tunserver params_list:"<<params_list;

	return params_list;
}

bool tunserverProcess::is_alive() {
	if(m_tunserver_process == nullptr) {
		qDebug()<<"Process is not defined (nullptr)";
		return false;
	}
	else if (m_tunserver_process->state() != QProcess::NotRunning) {
		qDebug()<<"Socket is not connected";
		return false;
	} else {
		return true;
	}
}
