#include <QDebug>

#include "commandsender.h"
#include "sendedcommand.h"

CommandSender::CommandSender( commandExecutor* executor,QObject *parent )
    : QObject( parent ),m_executor( executor ),m_counter( QString( "fla" ) )
{
    m_sended = new SendedCommands( this );
    startCommunication();
}

void CommandSender::startCommunication()
{
    auto get_rpc_name = prepareCommand( orderType::GETNAME );
    m_executor->sendNetRequest( *get_rpc_name );
    m_wait_name_flag = true;
}

void CommandSender::onGetName( const QString& client_name )
{
    m_client_name = client_name;
    m_counter.setRpcName( m_client_name );
    m_wait_name_flag = false;
    sendCommand( CommandSender::orderType::GET_GALAXY_IPV6 );
}

std::shared_ptr<order> CommandSender::getOrder( const QString &rpc_num )
{
    return m_sended->getOrder( rpc_num );
}

void CommandSender::sendCommand( orderType type, const MeshPeer &peer )
{
    if( m_wait_name_flag ) {
        throw std::runtime_error ( tr( "can't send msg while witing for rpc name of client" ).toStdString() );
    }

    try {
        auto ord = prepareCommand( type,peer );
        m_executor->sendNetRequest( *ord );
        m_sended->addOrder( ord,QString::fromStdString( ord->getId() ) );
    } catch( std::runtime_error &e ) {
        qDebug()<< "exception while sending :" << e.what();
    }

}

void CommandSender::sendCommand( CommandSender::orderType type )
{
    if( m_wait_name_flag && type != orderType::GETNAME ) {
        throw std::runtime_error ( tr( "can't send msg while witing for rpc name of client" ).toStdString() );
//        return;
    }

    try {
        auto ord = prepareCommand( type );
        if (ord == nullptr) {
            throw std::runtime_error("can't create order type" );
        }
        m_executor->sendNetRequest( *ord );
        m_sended->addOrder( ord,QString::fromStdString( ord->getId() ) );
    } catch( std::runtime_error &e ) {
        qDebug()<< "exception while sending :" << e.what();
    }
}

std::shared_ptr<order> CommandSender::prepareCommand( CommandSender::orderType type )
{
    std::shared_ptr<order> ord;

    if ( type != orderType::GETNAME && m_client_name.size() == 0 ) {
            throw std::runtime_error ( tr( "no client name provided - can't send command" ).toStdString() );
    }

    switch ( type ) {
    case orderType::PING:
        ord = std::make_shared<pingOrder>( m_counter.getRpcId() );
        break;
    case orderType::DELETEALL:
        ord = std::make_shared<deleteAllPeersOrder>( m_counter.getRpcId() );
        break;
    case orderType::BANALL:
        ord = std::make_shared<banAllOrder> ( m_counter.getRpcId() );
        break;
    case orderType::GETNAME:
        ord = std::make_shared<getClientName> ();
        break;
    case orderType::GET_STATUS:
        ord = std::make_shared<statusOrder>(m_counter.getRpcId());
        break;
    case orderType::GET_GALAXY_IPV6:
        ord = std::make_shared<getGalaxyIpV6Order>(m_counter.getRpcId());
        break;
    default:
        throw std::runtime_error ( tr( "improper number of parameters for this function" ).toStdString() );
        break;
    }
    return ord;
}

std::shared_ptr<order> CommandSender::prepareCommand( CommandSender::orderType type, const MeshPeer &peer )
{
    std::shared_ptr<order> ord;
        if( m_client_name.size() == 0 ) throw std::runtime_error ( tr( "no client name provided - can't send command" ).toStdString() );

    switch ( type ) {
    case orderType::ADDPEER:
        ord = std::make_shared<addPeerOrder> ( m_counter.getRpcId(),peer );
        break;
    case orderType::DELETEPEER:
        ord = std::make_shared<deletePeerOrder> ( m_counter.getRpcId(),peer );
        break;
    case orderType::BANPEER:
        ord = std::make_shared<banPeerOrder> ( m_counter.getRpcId(),peer );
        break;
    case orderType::PAY:{
        uint32_t pay = peer.getToPay();
        ord = std::make_shared<payOrder> ( m_counter.getRpcId(),peer ,pay);
        }
        break;
    case orderType::SET_ACCOUNT:
        ord = std::make_shared<setAccountOrder> ( m_counter.getRpcId(),peer );
        break;
    default:
        throw std::runtime_error ( tr( "can't create order with this params list" ).toStdString() );
    }

    return ord;
}
