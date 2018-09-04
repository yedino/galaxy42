#include <QDebug>

#include "order.hpp"
#include "mainwindow.hpp"
#include "nodecontrolerdialog.h"


std::string setIps::get_str() const
{
    nlohmann::json j{{"cmd", m_cmd}, {"msg",m_ip_list}};
    return j.dump();
}

setIps::setIps( const RpcId& id,const std::vector<std::string> &ip_list ):order( id ),m_ip_list( ip_list )
{
    m_cmd = "get_galaxy_new_format_reference";
    m_state = "ok";
    m_id =id.m_id;
}

setIps::setIps( const std::string &json_str ):order( json_str )
{
    using nlohmann::json;
    try {
        json j = json::parse( json_str );
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
    } catch( std::exception &e ) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void setIps::execute( MainWindow &main_window )
{
    if ( m_state =="error" ) { //if contains error
        main_window.errorNotification( QString::fromStdString( m_msg ) );
    } else {
        main_window.addDebugInfo( QString::fromStdString( m_msg ) );
        main_window.onGetMyInvitatiom( m_msg );
    }
}

basicOrder::basicOrder( const std::string &json_str ): order( json_str )
{
    try{
        using nlohmann::json;
        json j = json::parse( json_str );
        m_cmd = j["cmd"];
        m_state = j["state"];
    }catch(std::exception &e){
        qDebug()<<"can't parse answer";
        m_cmd = "unknown";
        m_state ="err";
    }
}

pingOrder::pingOrder( const RpcId& id ):order( id )
{
    m_cmd ="ping";
    m_msg = "ping";
    m_state = "ok";
    m_id = id.m_id;
}

pingOrder::pingOrder( const std::string &json_str ): order( json_str )
{
    using nlohmann::json;
    /*
        Request: {"cmd":"ping","msg":"ping","state":"ok"}
        Response: {"cmd":"ping","msg":"pong","state":"ok"}
    */
    try {
        json j = json::parse( json_str );
        m_cmd = j["cmd"];
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
//        m_state = j["state"];
    } catch ( std::exception &e ) {
        qDebug()<<"set ping parse error"<<json_str.c_str();
    }
}

void pingOrder::execute( MainWindow &mw )
{
//   mw.addDebugInfo(QString::fromStdString(m_msg));
    if( m_state != "ok" ) {
        mw.errorNotification( QString::fromStdString( m_msg ) );
    } else {
        mw.addDebugInfo( QString::fromStdString( m_msg ) );
    }
    return;
}

//peerListOrder::peerListOrder()
//{
//    m_cmd = "peer_list";
//    m_state = "ok";
//}

peerListOrder::peerListOrder( const std::string &json_str ): order( json_str )
{
    using nlohmann::json;
    try {
        json j = json::parse( json_str );
        m_cmd = j["cmd"];
        std::vector<std::string> tmp = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
        m_msg_array = std::move( tmp );
    } catch( std::exception &e ) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void peerListOrder::execute( MainWindow &main_window )
{
    if( m_state == "ok" ) {
        main_window.show_peers( m_msg_array );
    } else {
        main_window.errorNotification( QString::fromStdString( m_msg ) );
    }

    return;
}

getGalaxyOrder::getGalaxyOrder( const RpcId &id, const std::vector<std::string> &ipv4_list )
{
    Q_UNUSED(id)
    m_cmd ="get_galaxy_new_format_reference";
    m_msg_array = ipv4_list;
}

getGalaxyOrder::getGalaxyOrder( const std::string &json_str ):order( json_str )
{
    using nlohmann::json;
    json j = json::parse( json_str );
    try {
        m_cmd = j["cmd"];
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
    } catch( std::exception &e ) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
//    m_msg_array = std::move(tmp);

}

void getGalaxyOrder::execute( MainWindow &main_window )
{
    main_window.onGetMyInvitatiom( m_msg );
    return;
}

std::string getGalaxyOrder::get_str() const
{
    nlohmann::json j;
    j["id"] = m_id;
    j["cmd"]= m_cmd;
    j["msg"] = m_msg_array;
    j["state"] = m_state;
    j["id"] = m_id;

    return j.dump();
}

//addPeerOrder::addPeerOrder(const &RpcId id,const MeshPeer& peer) : order(id)
addPeerOrder::addPeerOrder( const RpcId& id,const MeshPeer& peer ):order( id )
{
    m_cmd = "add_peer";
    m_format = "1.0";
    m_id = id.m_id;
    QString tmp = peer.getVip() +"@(udp:" +peer.getIp() +":9042)";
    m_peer = tmp.toStdString();
}

addPeerOrder::addPeerOrder( const std::string &json_str,commandExecutor * executor ) : order( json_str )
{
    try {
        using nlohmann::json;
        json j = json::parse( json_str );
        m_executor = executor;
        m_msg = j["msg"];
        m_state = j["state"];
        m_id = j["id"];
        m_re = j["re"];
        if( m_state == "ok" ) {
            if( j.find( "peer" ) != j.end() ) {
                m_peer = j["peer"];
            }
        }
    } catch( std::exception &e ) {
        qDebug()<<"add peers parse error"<<json_str.c_str();
    }
}

std::string addPeerOrder::get_str() const
{
    nlohmann::json j{{"id",m_id},{"cmd", m_cmd}, {"format", m_format}, {"peer",m_peer}, {"state",m_state}};

    return j.dump();
}

void addPeerOrder::execute( MainWindow &main_window )
{
    if( m_state.find( "error" ) != std::string::npos ) { //if contains error
        main_window.errorNotification( QString::fromStdString( m_msg ) );
    } else {
        try {
            std::string peer = getPeerName();
            main_window.onPeerAdded( QString::fromStdString( peer ) );
        } catch( std::bad_cast &e ) {
            qDebug()<<e.what();
        }
    }
}


banPeerOrder::banPeerOrder( const RpcId& id,const MeshPeer &peer ):order( id )
{
    m_cmd = "ban_peer";
    m_state = "ok";
    m_peer = peer.getVip().toStdString();

//    m_peer = peer.getVip();
}

banPeerOrder::banPeerOrder( const std::string &json_str,commandExecutor* exec ):order()
{
    /*
        Ban peer

        Request: {"cmd":"ban_peer","peer":"<ipv6>","state":"ok"}
        Response: {"cmd":"ban_peer","msg":"ok: Peer banned","state":"ok"}
        or
        Response: {"cmd":"ban_peer","msg":"fail: Bad peer format","state":"error"}
    */
    m_executor = exec;
    try {
        using nlohmann::json;
        json j = json::parse( json_str );

        m_msg = j["msg"];
        m_state = j["state"];

        if( m_state == "ok" ) {
            if( j.find( "peer" ) != j.end() ) {
                m_peer = j["peer"];
            }
            m_re = j["re"];
        }

    } catch( std::exception &e ) {
        qDebug()<<"ban peers parse error"<<json_str.c_str();
    }
}

std::string banPeerOrder::get_str() const
{

    nlohmann::json j{{"cmd", m_cmd}, {"peer",m_peer}, {"state",m_state},{"id",m_id}};
    return j.dump();
}

void banPeerOrder::execute( MainWindow &main_window )
{
    try {
        QString peer = QString::fromStdString(getPeerName());
//        main_window.onBanBeer(peer);
        main_window.onPeerBanned(peer);
    } catch ( std::exception &e ) {
        qDebug()<<e.what();
    }
}


banAllOrder::banAllOrder( const RpcId& id ): order( id )
{
    m_cmd = "ban_all_peers";
}

banAllOrder::banAllOrder( const std::string &json_str ): order( json_str )
{
    try {
        using nlohmann::json;
        json j = json::parse( json_str );
        m_msg = j["msg"];
        m_state = j["state"];
    } catch( std::exception &e ) {
        qDebug()<<"set Ips parse error"<<json_str.c_str();
    }
}

void banAllOrder::execute( MainWindow &main_window )
{
    if( m_msg.find( "bad" ) != std::string::npos ) {
        main_window.errorNotification( QString::fromStdString( m_msg ) );
    } else {
        main_window.addDebugInfo( QString::fromStdString( m_msg ) );
    }
}


std::string banAllOrder::get_str() const
{
    nlohmann::json j{{"cmd", m_cmd}, {"state",m_state},{"id",m_id}};
    return j.dump();
}

deletePeerOrder::deletePeerOrder( const RpcId& id,const MeshPeer &peer )
{

    m_cmd = "delete_peer";
    m_state = "ok";
    m_peer = peer.getVip().toStdString();
    m_id = id.m_id;

}

deletePeerOrder::deletePeerOrder( const std::string &json_str,commandExecutor *exec ):order()
{
    try {
        using nlohmann::json;
        json j = json::parse( json_str );
        m_executor = exec;
        m_msg = j["msg"];
        m_state = j["state"];
        m_re = j["re"];
    } catch( std::exception &e ) {
        qDebug()<<"delete peer order problem:"<<json_str.c_str()<<" :"<<e.what();
    }
}

void deletePeerOrder::execute( MainWindow &main_window )
{
    if( m_state == "ok" ) {
        try {
            QString peer =QString::fromStdString( getPeerName());
            main_window.onDeletePeer(peer);
        } catch( std::exception &e ) {
            qDebug()<<e.what();
        }
//        main_window.addDebugInfo(QString::fromStdString(m_msg));
//           main_window.onPeerRemoved(QString::fromStdString(m_peer));
    } else {

//      main_window.errorNotification(QString::fromStdString(m_msg));
    }
}

std::string deletePeerOrder::get_str() const
{
    nlohmann::json j;
    j["cmd"]= m_cmd;
    j["peer"] = m_peer;
    j["state"] = m_state;
//    j["rpc"]
    return j.dump();

}

deleteAllPeersOrder::deleteAllPeersOrder( const RpcId& id ):order( id )
{
    m_cmd = "delete_all_peers";
    m_state = "ok";
}

void deleteAllPeersOrder::execute( MainWindow &main_window )
{
    if( m_msg.find( "error:" ) != std::string::npos ) {
        main_window.errorNotification( QString::fromStdString( m_msg ) );
    } else {
        main_window.addDebugInfo( QString::fromStdString( m_msg ) );
    }
    return;
}

std::string deleteAllPeersOrder::get_str() const
{
    nlohmann::json j{{"cmd", m_cmd}, {"state",m_state},{"id",m_id}};
    return j.dump();
}


order::order( const std::string &json_str )
{
    using nlohmann::json;
    json j = json::parse( json_str );
    try {
//        m_msg = j["msg"];
        m_state = j["state"];
    } catch ( std::exception &e ) {
        qDebug()<<e.what();
    }
}

order::order( order::e_type cmd ) {
    m_state = "ok";

    if ( cmd == e_type::PING ) {
        m_cmd = "ping";
        m_msg = "ping";

    }
    else if ( cmd == e_type::PEER_LIST ) {
        m_cmd = "peer_list";
    } else if( cmd == e_type::GET_INVITATION ) {
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

std::string order::getPeerName()
{
    if( !m_peer.empty() ) {
        return m_peer;
    }

    if( m_executor == nullptr ) {
        throw std::runtime_error ( " no known executor" );
    }

    auto ord_ptr =  m_executor->getOrder( QString::fromStdString( m_re ) );
    auto ord = ord_ptr.get();
    return ord->m_peer;

}

getClientName::getClientName( const std::string &json_str,commandExecutor *executor )
{
	nlohmann::json j = nlohmann::json::parse( json_str );
	m_state = j["state"];
	m_id = j["id"];
//	if(m_state == "ok") {
//		m_account = j["account_address"];
//	}
	m_executor = executor;
}

std::string getClientName::get_str() const
{
    nlohmann::json j{{"cmd",m_cmd}, {"state",m_state}};
    return j.dump();
}

void getClientName::execute( MainWindow & )
{
    if( m_state == "ok" ) {
        QString id = QString::fromStdString( m_id );
        QString client_num= id.split( "-" ).at( 0 );
        client_num += "-cli";
        assert(m_executor != nullptr);
        m_executor->setSenderRpcName( client_num );
    } else  {
//        main_window.resetCommunication;
        qDebug()<< "can't get name ";
    }
}

serverMsg::serverMsg(const std::string &json_str)
{
    try{
        nlohmann::json j = nlohmann::json::parse( json_str );
        m_state = j["state"];
        m_msg = j["msg"];
        m_lvl = j["lvl"];

    } catch (std::exception &e) {
        qDebug()<<e.what();
    }
}

void serverMsg::execute(MainWindow &)
{
    qDebug()<<QString::fromStdString(m_state) <<" "<< QString::fromStdString(m_lvl)<<" "<<QString::fromStdString(m_msg);
    //!@todo use app debug system while exists
}

statusOrder::statusOrder(const std::string &json_str,commandExecutor *executor)
{
    if(executor == nullptr){
        throw std::runtime_error("no executor error");
    }
    try{
        nlohmann::json j = nlohmann::json::parse( json_str );
        m_state = j["state"];
        m_satoshi = j["btc"];
    } catch (std::exception &e) {
        qDebug()<< "parser error while reding: "<<json_str.c_str();
    }
}

void payOrder::execute(MainWindow &main_window)
{
    std::string name = getPeerName();	//only for clear from list
//    __UNUSED_PARAM(name);

    if(m_state != "ok") {
        qDebug()<<"failure while pay: "<<name.c_str();

        main_window.setDebugInfo("can't execute");
        return;
    }

    return;
}

std::string payOrder::get_str() const
{
    nlohmann::json j{{"cmd",m_cmd}, {"state",m_state},{"id",m_id},{"btc",m_satoshi},{"peer",m_peer}};
    return j.dump();
}

payOrder::payOrder(const std::string &json_str,commandExecutor *executor) {
    m_executor = executor;
    try{
        nlohmann::json j = nlohmann::json::parse( json_str );
        m_state = j["state"];
        m_id = j["id"];

        if(m_state == "error"){
            m_msg = j["msg"];
            m_satoshi = 0;
        }else{
            m_satoshi = j["btc"];
        }

    }catch(std::exception& e ) {
        qDebug()<<"can't parse json";
    }
}

payOrder::payOrder(const RpcId& id,const MeshPeer &peer ,int satoshi)
{
    m_cmd = "pay";
    m_id = id.m_id;
    m_peer = peer.getVip().toStdString();
    m_satoshi = satoshi;
}

void statusOrder::execute(MainWindow &main_window)
{
    if(m_state == "ok") main_window.setBtc(m_satoshi);
//    else main_window.setDebugInfo(m_msg);
}

std::string statusOrder::get_str() const
{
    nlohmann::json j{{"cmd",m_cmd}, {"state",m_state},{"id",m_id}};
    return j.dump();
}


setAccountOrder::setAccountOrder(const RpcId& id,const MeshPeer &peer )
{
    m_cmd = "set_account";
    m_id = id.m_id;
    m_state = "ok";
    m_peer = peer.getVip().toStdString();
    m_account = peer.getAccount().toStdString();
}

setAccountOrder::setAccountOrder(const std::string &json_str,commandExecutor *executor)
{
    try{
        nlohmann::json j = nlohmann::json::parse( json_str );
        m_state = j["state"];
        m_account = j["account"];
        m_id = j["id"];
    }catch(std::exception &e){
        qDebug()<<e.what();
        return;
    }
}

void setAccountOrder::execute(MainWindow &main_window)
{
     m_peer = getPeerName();
}

std::string setAccountOrder::get_str() const
{
    nlohmann::json j{{"cmd",m_cmd}, {"state",m_state}, {"id",m_id},{"peer",m_peer} ,{"account",m_account}};
    return j.dump();
}


statusOrder::statusOrder(const RpcId& Id)
{
    try{
        m_cmd ="get_status";
        m_state = "ok";
        m_id = Id.m_id;
    }catch(std::exception &e){
        qDebug()<<e.what();
    }
}

getGalaxyIpV6Order::getGalaxyIpV6Order(const RpcId& Id) 
{
    try{
        m_cmd ="get_galaxy_ipv6";
        m_state = "ok";
        m_id = Id.m_id;
    }catch(std::exception &e){
        qDebug()<<e.what();
    }
}

getGalaxyIpV6Order::getGalaxyIpV6Order(const std::string &json_str)
{
    try{
        nlohmann::json j = nlohmann::json::parse( json_str );
        m_state = j["state"];
        m_ipv6 = j["ipv6"];
        m_id = j["id"];
    }catch(std::exception &e){
        qDebug()<<e.what();
        return;
    }
}

void getGalaxyIpV6Order::execute(MainWindow &main_window)
{
	if( m_state == "ok" ) {
		QString ipv6 = QString::fromStdString(m_ipv6);
		main_window.setIps( ipv6, ipv6 );
	} else  {
		qDebug()<< "can't get address ip V6 ";
	}
}


