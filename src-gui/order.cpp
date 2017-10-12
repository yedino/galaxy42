#include "order.hpp"
#include "mainwindow.hpp"
#include "nodecontrolerdialog.h"

std::string setIps::get_str() const
{
        nlohmann::json j{{"cmd", m_cmd} , {"msg",m_ip_list}};
        return j.dump();
}

setIps::setIps(const RpcId& id,const std::vector<std::string> &ip_list):order(id) ,m_ip_list(ip_list)
{
    m_cmd = "get_galaxy_new_format_reference";
    m_state = "ok";
    m_id =id.m_id;
}

setIps::setIps(const std::string &json_str):order(json_str)
{
    using nlohmann::json;
    try {
        json j = json::parse(json_str);
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
    } catch(std::exception &e) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void setIps::execute(MainWindow &main_window)
{
    if (m_state =="error") { //if contains error
        main_window.errorNotification(QString::fromStdString(m_msg));
    } else {
        main_window.addDebugInfo(QString::fromStdString(m_msg));
        main_window.onGetMyInvitatiom(m_msg);
    }
}

basicOrder::basicOrder(const std::string &json_str): order(json_str)
{
    using nlohmann::json;
    json j = json::parse(json_str);
    m_cmd = j["cmd"];
    m_state = j["state"];
}

pingOrder::pingOrder(const RpcId& id):order(id)
{
    m_cmd ="ping";
    m_msg = "ping";
    m_state = "ok";
    m_id = id.m_id;
}

pingOrder::pingOrder(const std::string &json_str): order(json_str)
{
    using nlohmann::json;
/*
    Request: {"cmd":"ping","msg":"ping","state":"ok"}
    Response: {"cmd":"ping","msg":"pong","state":"ok"}
*/
    try {
        json j = json::parse(json_str);
        m_cmd = j["cmd"];
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
//        m_state = j["state"];
    } catch (std::exception &e) {
        qDebug()<<"set ping parse error"<<json_str.c_str();
    }
}

void pingOrder::execute(MainWindow &mw)
{
//   mw.addDebugInfo(QString::fromStdString(m_msg));
    if(m_state != "ok"){
        mw.errorNotification(QString::fromStdString(m_msg));
    } else {
        mw.addDebugInfo(QString::fromStdString( m_msg));
    }
    return;
}

//peerListOrder::peerListOrder()
//{
//    m_cmd = "peer_list";
//    m_state = "ok";
//}

peerListOrder::peerListOrder(const std::string &json_str): order(json_str)
{
    using nlohmann::json;
    try {
        json j = json::parse(json_str);
        m_cmd = j["cmd"];
        std::vector<std::string> tmp = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
        m_msg_array = std::move(tmp);
    } catch(std::exception &e) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void peerListOrder::execute(MainWindow &main_window)
{
    if(m_state == "ok") {
        main_window.show_peers(m_msg_array);
    } else {
        main_window.errorNotification(QString::fromStdString(m_msg));
    }

    return;
}

getGalaxyOrder::getGalaxyOrder(const RpcId &id, const std::vector<std::string> &ipv4_list)
{
    m_cmd ="get_galaxy_new_format_reference";
    m_msg_array = ipv4_list;
}

getGalaxyOrder::getGalaxyOrder(const std::string &json_str):order(json_str)
{
    using nlohmann::json;
    json j = json::parse(json_str);
    try {
        m_cmd = j["cmd"];
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
    } catch(std::exception &e) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
//    m_msg_array = std::move(tmp);

}

void getGalaxyOrder::execute(MainWindow &main_window)
{
    main_window.onGetMyInvitatiom(m_msg);
    return;
}

std::string getGalaxyOrder::get_str() const
{
    nlohmann::json j;
    j["id"] = m_id;
    j["cmd"]= m_cmd;
    j["msg"] = m_msg_array;
    j["state"] = m_state;


    return j.dump();
}

//addPeerOrder::addPeerOrder(const &RpcId id,const MeshPeer& peer) : order(id)
addPeerOrder::addPeerOrder(const RpcId& id,const MeshPeer& peer):order(id)
{
    m_cmd = "add_peer";
    m_format = "1.0";
    m_id = id.m_id;
    QString tmp = peer.getVip() +"@(udp:" +peer.getIp() +":9042)";
    m_peer = tmp.toStdString();
}

addPeerOrder::addPeerOrder(const std::string &json_str,commandExecutor * executor) : order(json_str)
{
    try {
        using nlohmann::json;
        json j = json::parse(json_str);
        m_executor = executor;
			m_re = j["re"];
		m_msg = j["msg"];
        m_state = j["state"];
		m_id = j["id"];
		if(m_state == "ok"){
//            m_format = j["format"];
//            m_peer = j["peer"];
        }

    } catch(std::exception &e) {
        qDebug()<<"add peers parse error"<<json_str.c_str();
    }
}

std::string addPeerOrder::get_str() const
{
    nlohmann::json j{{"id",m_id},{"cmd", m_cmd} , {"format", m_format} , {"peer",m_peer} , {"state",m_state}};

    return j.dump();
}

void addPeerOrder::execute(MainWindow &main_window)
{
    if(m_state.find("error") != std::string::npos){ //if contains error
		main_window.errorNotification(QString::fromStdString(m_msg));
    }else{
        auto ord_ptr =  m_executor->getOrder(QString::fromStdString(m_re));
        auto ord = ord_ptr.get();

       try {
            addPeerOrder* add_ord = dynamic_cast<addPeerOrder*>(ord);
            main_window.onPeerAdded(QString::fromStdString( add_ord->m_peer ));
       } catch(std::bad_cast &e) {
            qDebug()<<e.what();
        }
   }
}


banPeerOrder::banPeerOrder(const RpcId& id,const MeshPeer &peer):order(id)
{
    m_cmd = "ban_peer";
    m_state = "ok";
    m_peer = peer.getVip().toStdString();
//    m_peer = peer.getVip();
}

banPeerOrder::banPeerOrder(const std::string &json_str)
{
/*
    Ban peer

    Request: {"cmd":"ban_peer","peer":"<ipv6>","state":"ok"}
    Response: {"cmd":"ban_peer","msg":"ok: Peer banned","state":"ok"}
    or
    Response: {"cmd":"ban_peer","msg":"fail: Bad peer format","state":"error"}
*/
    try {
        using nlohmann::json;
        json j = json::parse(json_str);

        m_msg = j["msg"];
        m_state = j["state"];

        if(m_state == "ok"){
            m_peer = j["peer"];
            m_re = j["re"];
        }

    } catch(std::exception &e) {
        qDebug()<<"add peers parse error"<<json_str.c_str();
    }

}

std::string banPeerOrder::get_str() const
{

    nlohmann::json j{{"cmd", m_cmd} , {"peer",m_peer} , {"state",m_state}};
    return j.dump();
}

void banPeerOrder::execute(MainWindow &main_window)
{

}


banAllOrder::banAllOrder(const RpcId& id): order(id)
{
    m_cmd = "ban_all_peer";
}

banAllOrder::banAllOrder(const std::string &json_str): order(json_str)
{
    try{
        using nlohmann::json;
        json j = json::parse(json_str);
        m_msg = j["msg"];
        m_state = j["state"];
    } catch(std::exception &e) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void banAllOrder::execute(MainWindow &main_window)
{
    if(m_msg.find("bad") != std::string::npos ) {
        main_window.errorNotification(QString::fromStdString(m_msg));
    }else {
        main_window.addDebugInfo(QString::fromStdString(m_msg));
    }
}

deletePeerOrder::deletePeerOrder(const RpcId& id,const MeshPeer &peer)
{

   m_cmd = "delete_peer";
   m_state = "ok";
   m_peer = peer.getVip().toStdString();
   m_id = id.m_id;

}

deletePeerOrder::deletePeerOrder(const std::string &json_str):order(json_str)
{
    try {
        using nlohmann::json;
        json j = json::parse(json_str);
        m_msg = j["msg"];
        m_state = j["state"];
        m_re = j["re"];

    } catch(std::exception &e) {
        qDebug()<<"delete peer order"<<json_str.c_str();
    }
}

void deletePeerOrder::execute(MainWindow &main_window)
{
    if(m_state == "ok" ) {
//        main_window.addDebugInfo(QString::fromStdString(m_msg));
//           main_window.onPeerRemoved(QString::fromStdString(m_peer));
    } else {
        main_window.errorNotification(QString::fromStdString(m_msg));
    }
}

std::string deletePeerOrder::get_str() const
{
    nlohmann::json j;
    j["cmd"]= m_cmd;
    j["peer"] = m_peer;
    j["state"] = m_state;
    return j.dump();
}

deleteAllPeersOrder::deleteAllPeersOrder(const RpcId& id):order(id)
{
    m_cmd = "delete_all_peer";
    m_state = "ok";
}

void deleteAllPeersOrder::execute(MainWindow &main_window)
{
    if(m_msg.find("error:") != std::string::npos ) {
        main_window.errorNotification(QString::fromStdString(m_msg));
    }else {
        main_window.addDebugInfo(QString::fromStdString(m_msg));
    }
    return;
}


order::order(const std::string &json_str)
{
        using nlohmann::json;
        json j = json::parse(json_str);
        try{
//        m_msg = j["msg"];
            m_state = j["state"];
        }catch (std::exception &e){
            qDebug()<<e.what();
        }
}

order::order(order::e_type cmd) {
    m_state = "ok";

    if (cmd == e_type::PING) {
        m_cmd = "ping";
        m_msg = "ping";

    }
    else if (cmd == e_type::PEER_LIST) {
        m_cmd = "peer_list";
    } else if(cmd == e_type::GET_INVITATION) {
        m_cmd = "get_ip";
    }
}

std::string order::get_str() const {
    nlohmann::json j;
    j["cmd"] = m_cmd;
    j["msg"] = m_msg;
    j["state"]  = m_state;

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



getClientName::getClientName(const std::string &json_str,commandExecutor *executor)
{
    try{
        nlohmann::json j = nlohmann::json::parse(json_str);
        m_state = j["state"];
        m_id = j["id"];
        m_executor = executor;
    }catch(std::exception &e){
        qDebug()<<e.what();
    }
}

std::string getClientName::get_str() const
{
    nlohmann::json j{{"cmd",m_cmd}, {"state",m_state}};
    return j.dump();
}

void getClientName::execute(MainWindow &)
{
    if(m_state == "ok") {
        QString id = QString::fromStdString(m_id);
        QString client_num= id.split("-").at(0);
        client_num += "-cli";
       m_executor->setSenderRpcName(client_num);
    } else  {
//        main_window.resetCommunication;
        qDebug()<< "can't get name ";
    }
}
