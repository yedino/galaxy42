#ifndef COMMANDSENDER_H
#define COMMANDSENDER_H

#include <QObject>

#include "meshpeer.h"
#include "order.hpp"
#include "rpccounter.h"
#include "commandexecutor.hpp"
#include "commandparser.hpp"
#include "sendedcommand.h"

class order;
class commandExecutor;
class SendedCommands;


class CommandSender : public QObject
{
    Q_OBJECT
public:

    enum class orderType
    {
        PING =0,
        ADDPEER,
        DELETEPEER,
        BANPEER,
        DELETEALL,
        BANALL,
        GETNAME,
        GET_STATUS,
        PAY,
        SET_ACCOUNT,
        GET_GALAXY_IPV6
    };

    explicit CommandSender(commandExecutor* comm_exec,QObject *parent = nullptr);

    void startCommunication();
    void sendCommand(orderType type);
    void sendCommand(orderType type,const MeshPeer &peer);

    std::shared_ptr<order> prepareCommand(CommandSender::orderType type);
    std::shared_ptr<order> prepareCommand(CommandSender::orderType type,
                                                        const MeshPeer &peer);
    void onGetName(const QString &client_name);

    std::shared_ptr<order>getOrder(const QString &rpc_num);

signals:

public slots:

private:

    QString m_client_name;
    commandExecutor* m_executor;
    RpcCounter m_counter;
    SendedCommands *m_sended;


    bool m_wait_name_flag = false;
//
//	commandExecutor& executor;
//
};

#endif // COMMANDSENDER_H
