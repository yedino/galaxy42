#include <memory>
#include <QSettings>
#include <QHostAddress>


#include "commandexecutor.hpp"
#include "commandsender.h"
#include "statusform.h"


std::shared_ptr<commandExecutor> commandExecutor::construct( std::shared_ptr<MainWindow> window ) {
    std::shared_ptr<commandExecutor>ret = std::make_shared<commandExecutor>( window );
//	ret->m_net_client = std::make_shared<netClient>(ret);
    ret->m_net_client = new netClient( ret.get() );

    return ret;
}

void commandExecutor::resetConnection()
{
//    m_net_client - disconnect
    m_net_client->closeConnection();
    emit Disconnected();
//    m_net_client->
    ;
//    sendNetRequest(m_sender->prepareCommand(CommandSender::orderType::GETNAME).get());
//    if(star)
    m_sender->sendCommand( CommandSender::orderType::GETNAME );
}


void commandExecutor::parseAndExecMsg( const std::string &msg ) {
    std::unique_ptr <order> ord;
    basicOrder inputOrder( msg );
    if( inputOrder.get_cmd() == "ping" ) {				//fragment fabryki - wytwarza obiekt odpowiedniej klasy ...
        ord = std::make_unique <pingOrder>( msg );
    } else if( inputOrder.get_cmd() == "peer_list" ) {
        ord = std::make_unique <peerListOrder>( msg );
    } else if( inputOrder.get_cmd() == "add_peer" ) {
        ord = std::make_unique <addPeerOrder>( msg,this );
    } else if( inputOrder.get_cmd() == "delete_peer" ) {
        ord = std::make_unique <deletePeerOrder>( msg,this );
    } else if( inputOrder.get_cmd() == "delete_all_peer" ) {
        ord = std::make_unique <deleteAllPeersOrder>( msg );
    } else if( inputOrder.get_cmd() == "ban_peer" ) {
        ord = std::make_unique<banPeerOrder>( msg,this );
    } else if( inputOrder.get_cmd() == "ban_all_peer" ) {
        ord = std::make_unique <banAllOrder>( msg );
    } else if( inputOrder.get_cmd() == "get_galaxy_ipv6" ) {
        ord = std::make_unique <basicOrder>( msg );
    } else if( inputOrder.get_cmd() == "get_galaxy_new_format_reference" ) {
        ord = std::make_unique <getGalaxyOrder>( msg );
    } else if( inputOrder.get_cmd() == "hello" ) {
        ord = std::make_unique<getClientName>( msg,this );
    } else {
        qDebug()<<"unknown command";
        return;
    }

    try {
        ord->execute( *m_main_window );				// a pozniej go wykonuje - nie jest thread safe
    } catch( std::runtime_error &e ) {
//        qDebug()<<e.what();
        emit ErrorOccured( QString( e.what() ) );
    }
}

void commandExecutor::sendNetRequest( const order &ord ) {

    try {
        if ( !m_net_client->is_connected() ) {
            emit Disconnected();
            QSettings settings;
            QString ip = settings.value( "rpcConnection/Ip" ).toString();
            int port = settings.value( "rpcConnection/port" ).toInt();
            if( m_net_client->startConnect( QHostAddress( ip ),port ) ) {
                emit Connected();
            }
        }
    } catch( std::exception& e ) {
        emit ErrorOccured( QString( e.what() ) );
    }

    try {
        m_net_client->send_msg( ord.get_str() );
    } catch ( std::runtime_error &e ) {
//            qDebug(e.what());
//        qDebug()<<e.what();
        emit ErrorOccured( e.what() );
    }
}

void commandExecutor::startConnect( const QHostAddress &address, uint16_t port ) {
    if( m_net_client->is_connected() ) {
        m_net_client->closeConnection();
    }
    if( m_net_client->startConnect( address, port ) ) {
        emit Connected();
    } else {
        emit ErrorOccured( tr( "can not connect to host" ) );
    }

}

commandExecutor::commandExecutor( MainWindow* win ):
    m_main_window( win ),
    m_net_client( nullptr ),
    m_timer( std::make_unique<QTimer>() )
{
    m_net_client = new netClient( this );
    connect( this,SIGNAL( ErrorOccured( QString ) ),win->GetStatusObject(),SLOT( onErrorOccured( QString ) ) );
    connect( this,SIGNAL( Connected() ),win->GetStatusObject(),SLOT( onConnectionSuccess() ) );
    connect( this,SIGNAL( Disconnected() ),win->GetStatusObject(),SLOT( onLostConnection() ) );
    connect( this,SIGNAL( GetSesionId() ),win->GetStatusObject(),SLOT( onGetSessionId() ) );

}

commandExecutor::commandExecutor( std::shared_ptr<MainWindow> window )
    :
    m_main_window( window.get() ),
    m_net_client( nullptr ),
    m_timer( std::make_unique<QTimer>() )
{
    m_net_client = new netClient( this );
    connect( m_timer.get(), SIGNAL( timeout() ), this, SLOT( timer_slot() ) );
    m_timer->start( 5000 );

    connect( this,SIGNAL( ErrorOccured( QString ) ),window->GetStatusObject(),SLOT( onErrorOccured( QString ) ) );
    connect( this,SIGNAL( Connected() ),window->GetStatusObject(),SLOT( onConnectionSuccess() ) );
    connect( this,SIGNAL( Disconnected() ),window->GetStatusObject(),SLOT( onLostConnection() ) );
    connect( this,SIGNAL( GetSesionId() ),window->GetStatusObject(),SLOT( onGetSessionId() ) );
}

void commandExecutor::timer_slot() {

//    std::string ip = "127.0.0.1";
//    std::vector<std::string> vect;
//    vect.push_back(ip);
//    pingOrder ord;
//    sendNetRequest(ord);
//    m_net_client->send_msg(ord.get_str());

    qDebug() << "timer slot";
}

void commandExecutor::setSender( CommandSender *sender )
{
    m_sender = sender;
}

void commandExecutor::setSenderRpcName( const QString& name )
{
    if( !m_sender ) {
        throw std::runtime_error( "no sender attached" );

    }

    emit GetSesionId();

    m_sender->onGetName( name );
}

std::shared_ptr<order> commandExecutor::getOrder( const QString &rpc_id )
{
    if( m_sender ) {
        return m_sender->getOrder( rpc_id );
    } else {
        throw std::runtime_error ( "no sender" );
    }
}
