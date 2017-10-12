#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include "mainwindow.hpp"
#include "netclient.hpp"
#include <memory>
#include <QTimer>
#include <vector>

//#include "../depends/json/src/json.hpp"
#include "order.hpp"
#include "commandsender.h"


class MainWindow;

class netClient;
class order;
class CommandSender;

class commandExecutor final : public QObject {
		Q_OBJECT

    public:
		static std::shared_ptr<commandExecutor> construct(std::shared_ptr<MainWindow> window);
		void parseAndExecMsg(const std::string &msg); ///< parse network msg
		void sendNetRequest(const order &ord); ///< send order via network
		void startConnect(const QHostAddress &address, uint16_t port);

        commandExecutor(std::shared_ptr<MainWindow> window);
        commandExecutor(MainWindow* win);

        void setSender(CommandSender *sender);
        void setSenderRpcName(const QString&name);

        std::shared_ptr<order> getOrder(const QString& rpc_id);

private:
        MainWindow* m_main_window;
        netClient* m_net_client;
        std::unique_ptr<QTimer> m_timer;
        CommandSender* m_sender = nullptr;
	private slots:
		void timer_slot();
};

#endif // COMMANDEXECUTOR_H

