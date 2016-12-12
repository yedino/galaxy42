#ifndef NETCLIENT_HPP
#define NETCLIENT_HPP

#include <memory>
#include <QTcpSocket>
#include "dataeater.hpp"
#include "commandexecutor.hpp"

class commandExecutor;

class netClient final : QObject {
		Q_OBJECT
	public:
		netClient(std::shared_ptr<commandExecutor> cmd_exec_ptr);
		void startConnect(const QHostAddress &address, uint16_t port);
		bool is_connected();
		void send_msg(const std::string &msg);

		static QByteArray serialize_msg(const std::string &msg);
	private:
		std::weak_ptr<commandExecutor> m_cmd_exec;
		std::unique_ptr<QTcpSocket> m_socket;
		dataeater m_data_eater;

	private slots:
		void onTcpReceive();
};

#endif // NETCLIENT_HPP
