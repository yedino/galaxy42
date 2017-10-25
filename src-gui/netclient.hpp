#ifndef NETCLIENT_HPP
#define NETCLIENT_HPP

#include <memory>
#include <QTcpSocket>
#include <sodium.h>
#include "dataeater.hpp"
#include "commandexecutor.hpp"

class commandExecutor;

class netClient final : QObject {
		Q_OBJECT
	public:
        netClient(commandExecutor* cmd_exec_ptr);
        bool startConnect(const QHostAddress &address, uint16_t port);
		bool is_connected();
		void send_msg(const std::string &msg);

        static QByteArray serialize_msg(const std::string &msg);
        void closeConnection();
private:
        commandExecutor* m_cmd_exec;
		std::unique_ptr<QTcpSocket> m_socket;
		dataeater m_data_eater;
		std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> m_hmac_key;
		/**
		 * @brief check_auth
		 * @param cmd_and_auth data + crypto_auth_hmacsha512_BYTES bytes of HMAC-SHA-512
		 * @return true if authentication pass
		 */
		bool check_auth(const std::string &cmd_and_auth);

	private slots:
		void onTcpReceive();
};

#endif // NETCLIENT_HPP
