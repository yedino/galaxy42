#include <QDebug>
#include <QProcess>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QSettings>
#include <QMessageBox>
#include <QErrorMessage>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "addressdialog.hpp"
#include "paramscontainer.hpp"
#include "dataeater.hpp"
#include "debugdialog.hpp"
#include "get_host_info.hpp"
#include "quickstartdialog.h"
#include "nodecontrolerdialog.h"
#include "statusform.h"

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    m_tun_process( std::make_unique<tunserverProcess>() )
{
    ui->setupUi( this );

    ui->peerListWidget_2->setMainWindow( this );

    QString ip = getLocalIps().at( 0 );
    QString vip = getLocalVips().at( 0 ).split( '%' ).at( 0 );
    ui->quickStart->setIps( ip,vip );
    connect( ui->quickStart,SIGNAL( connectNet( QString ) ),this,SLOT( connectToNet( QString ) ) );
    connect( ui->quickStart,SIGNAL( createNet() ),this,SLOT( createNet() ) );
    connect( ui->quickStart, SIGNAL( allowFriend( bool ) ),this,SLOT( onAllowFriend( bool ) ) );
    connect( ui->quickStart, SIGNAL( allowStranger( bool ) ),this,SLOT( onAllowPeer( bool ) ) );


    connect( ui->peerListWidget_2,SIGNAL( addPeer( QString ) ),this,SLOT( onAddPeer( QString ) ) );
    connect( ui->peerListWidget_2,SIGNAL( removePeer( QString ) ),this,SLOT( onRemovePeer( QString ) ) );
    connect( ui->peerListWidget_2,SIGNAL( banPeer( QString ) ),this,SLOT( onBanBeer( QString ) ) );
    connect( ui->peerListWidget_2,SIGNAL( deleteAll() ),this,SLOT( onDeleteAll() ) );
    connect( ui->peerListWidget_2,SIGNAL( banAll() ),this,SLOT( onBanAll() ) );

    initSettings();
    m_status_form = new StatusForm( this );

    m_cmd_exec = new commandExecutor( this ) ;
    m_sender = new CommandSender( m_cmd_exec,this );
    m_cmd_exec->setSender( m_sender );
    loadSettings();

    ui->statusBar->addPermanentWidget( m_status_form );

    connect( m_status_form,SIGNAL( netConnect( bool ) ),this,SLOT( onNetConnected( bool ) ) );

    m_status_form->setExecutor( m_cmd_exec );
    isWorking();
}


void MainWindow::startNewCrpcConnection( const QString &host,uint port )//!@todo move to statusForm
{
    m_cmd_exec = new commandExecutor( this ) ;
    m_sender = new CommandSender( m_cmd_exec,this );
    m_cmd_exec->setSender( m_sender );
    m_cmd_exec->startConnect( QHostAddress( host ), port );
    m_sender->sendCommand( CommandSender::orderType::GETNAME );
}

std::shared_ptr<MainWindow> MainWindow::create_shared_ptr() {
    std::shared_ptr<MainWindow> ret = std::make_shared<MainWindow>();
//	ret->m_cmd_exec = commandExecutor::construct(ret);
    return ret;
}

void MainWindow::showDebugPage( QByteArray &pageCode ) {
    Q_UNUSED( pageCode );
}

void MainWindow::on_plusButton_clicked() {
    m_dlg = std::make_unique<addressDialog>( this );
    connect ( m_dlg.get(),SIGNAL( add_address( QString ) ),this,SLOT( update_peer_list( QString ) ) );
    m_dlg->show();
}

void MainWindow::update_peer_list( QString peer ) {

    m_tun_process->add_address( peer );

    qDebug() << "Peers:";
    for ( const auto &peer : m_tun_process->get_peer_list() ) {
        //qDebug() << peer.to_string();
//		ui->peerListWidget->addItem(QString::fromStdString(peer.to_string()));
    }
}

void MainWindow::on_minusButton_clicked() {
    /*
    	const auto &delete_list = ui->peerListWidget->selectedItems();
    	if(delete_list.isEmpty()) return;
    	try{
    		// double validating
    		m_tun_process->del_peer(peer_reference::get_validated_ref(delete_list.at(0)->text().toStdString()));

    		ui->peerListWidget->removeItemWidget(delete_list.at(0));
            delete_list.at(0)->setText("");
            ui->peerListWidget->sortItems(Qt::DescendingOrder);
        } catch(...){
            qDebug() << "list is empty";
        }
        */
}

void MainWindow::on_run_tunserver_clicked() {
    m_tun_process->run();
}

void MainWindow::add_host_info( QString host, uint16_t port )
{

    qDebug() << "Host: " << host << "Port:" << port << '\n';
    if ( host.isEmpty() ) {
        host = "127.0.0.1";
        qDebug() << "use default 'localhost (127.0.0.1)' host";
    }
    if ( port == 0 ) {
        port = 42000;
        qDebug() << "use default '42000' port";
    }

    QSettings settings;

    settings.setValue( "rpcConnection/Ip",host );
    settings.setValue( "rpcConnection/port",port );
    startNewCrpcConnection( host,port );
}

void MainWindow::on_connectButton_clicked() {

    if( ! isWorking() ) {
        QErrorMessage message( this );
        message.showMessage( tr( "can't connect to node" ) );
        message.exec();
        return;
    }
}


void MainWindow::connectToNet( QString net_id )
{
    if( ! isWorking() ) {
        QErrorMessage message( this );
        message.showMessage( tr( "can't connect to node" ) );
        message.exec();
        return;
    }

    try {

        MeshPeer peer;
        peer.setName( "quick add" );
        peer.setInvitation( net_id );
        m_sender->sendCommand( CommandSender::orderType::ADDPEER,peer );

    } catch( std::exception &e ) {
        qDebug()<<e.what();
    }

}

void MainWindow::createNet()
{
    try {
        onCreateGalaxyConnection();
    } catch( std::exception &e ) {
        qDebug()<<e.what();
    }

}

void MainWindow::deletePeer( const std::string &peer_id ) {
    try {
        MeshPeer peer;
        peer.setVip( QString::fromStdString( peer_id ) );
        m_sender->sendCommand( CommandSender::orderType::DELETEPEER,peer );
    } catch( std::exception &e ) {
        qDebug()<<e.what();
    }
}

void MainWindow::on_ping_clicked() {
    m_sender->sendCommand( CommandSender::orderType::PING );
}

// not used yed
void MainWindow::SavePeers( QString file_name ) {
    ParamsContainer container;
    //container.setPeerList(m_peer_lst);
    container.writeParams( file_name );
}

void MainWindow::add_to_debug_window( const std::string &message ) {
//	ui->debugWidget->addItem(message.c_str());
//	ui->debugWidget->scrollToBottom();
}

void MainWindow::show_peers( const std::vector<std::string> &peers ) {
//	ui->peerListWidget->clear();
    for ( const auto &element : peers ) {
//		ui->peerListWidget->addItem(QString(element.c_str()));
    }
}

void MainWindow::on_actionDebug_triggered() {
    qDebug()<< "show dialog";
    DebugDialog dialog;
    dialog.exec();
    dialog.show();
}

void MainWindow::onCreateGalaxyConnection()
{

    QStringList list_ips = getLocalIps();

    std::vector <std::string> ips;

    for ( auto &it:list_ips ) {
        ips.push_back( it.toStdString() );
    }
//    getGalaxyOrder ord(ips);
//    m_cmd_exec->sendNetRequest(ord);


//	getGalaxyOrder order
}


QStringList MainWindow::getLocalIps()
{
    QStringList ret_val;

    foreach ( const QHostAddress &address, QNetworkInterface::allAddresses() ) {
        if ( address.protocol() == QAbstractSocket::IPv4Protocol && address
                != QHostAddress( QHostAddress::LocalHost ) && !address.isLoopback() )
            ret_val.push_back( address.toString() );
    }

    return ret_val;
}

QStringList MainWindow::getLocalVips()
{
    QStringList ret_val;
    foreach ( const QHostAddress &address, QNetworkInterface::allAddresses() ) {
        if ( address.protocol() == QAbstractSocket::IPv6Protocol && address
                != QHostAddress( QHostAddress::LocalHost ) && !address.isLoopback() )
            ret_val.push_back( address.toString() );
    }
    return ret_val;
}

void MainWindow::onGetMyInvitatiom( std::string ipv6 )
{
    ui->quickStart->setIps( "aaa",QString::fromStdString( ipv6 ) );
}

void MainWindow::errorNotification( QString err )
{
//    ui->debugWidget->addItem(err);
    static bool error_flag = false;
    if ( error_flag ) {
        return ;
    }

    error_flag = true;

    QMessageBox::warning( this,tr( "Error" ),err );

    error_flag = false;
}

void MainWindow::addDebugInfo( const QString &str )
{
//    ui->debugWidget->addItem(str);
}

void MainWindow::loadSettings()
{
    QSettings setings;
    QString ip = setings.value( "rpcConnection/Ip" ).toString();
    QString port = setings.value( "rpcConnection/port" ).toString();
    if ( port.size()== 0 ) port="42000";			//! @todo add ability of changing port
    if( ip.size() == 0 ) ip="127.0.0.1";			//localhost
    m_host_port = port;
    m_host_ip = ip;

    qDebug()<<"ip & port"<<ip<<" "<<port;

}

void MainWindow::initSettings()
{
    QSettings setings;
    if( setings.allKeys().size() >= 2 ) {			//no need to initialize
        return;
    }

    setings.beginGroup( "rpcConnection" );
    setings.setValue( "connectionsNum","1" );		// no of known nodes
    setings.setValue( "Ip","127.0.0.1" );				// Ip of rpc node
    setings.setValue( "port","42000" );				// port of rpc node
    setings.endGroup();

    setings.beginGroup( "gui/mainWindow" );
    setings.endGroup();
}

void MainWindow::onAllowFriend( bool val )
{
    //! @todo implement allow firends

    if( ! isWorking() ) {
        QErrorMessage message( this );
        message.showMessage( tr( "can't connect to node" ) );
        message.exec();
        return;
    }

}

void MainWindow::onAllowPeer( bool val )
{
    //! @todo implement allow strangers
    if( ! isWorking() ) {
        QErrorMessage message( this );
        message.showMessage( tr( "can't connect to node" ) );
        message.exec();
        return;
    }
}

void MainWindow::on_banButton_clicked()
{
//    QList<QListWidgetItem> items = ui->peerListWidget->selectedItems();

//    items

//    banAllOrder ord;


    /*
    const auto &delete_list = ui->peerListWidget->selectedItems();
    if(delete_list.isEmpty()) return;
    try{
        // double validating
        m_tun_process->del_peer(peer_reference::get_validated_ref(delete_list.at(0)->text().toStdString()));

        ui->peerListWidget->removeItemWidget(delete_list.at(0));
        delete_list.at(0)->setText("");
        ui->peerListWidget->sortItems(Qt::DescendingOrder);
    } catch(...){
        qDebug() << "list is empty";
    }
    */

}

void MainWindow::onAddPerrToList( const QString &peer_string )
{
//    MeshPeer peer(peer_string.toStdString());
//    addPeerOrder ord;
//    addPeerOrder ord(peer);
//    m_cmd_exec->sendNetRequest(ord);
//    m_sender->sendCommand(CommandSender::orderType::ADDPEER,peer);
}

void MainWindow::onBanBeer( const QString& vip )
{
    try {
        MeshPeer peer;
        peer.setVip( vip );
        m_sender->sendCommand( CommandSender::orderType::BANPEER,peer );
    } catch( std::exception &e ) {
        qDebug()<<"problem whie order execution";
    }
}

void MainWindow::onRemovePeer( const QString& vip )
{
    try {
        MeshPeer peer;
        peer.setVip( vip );
        m_sender->sendCommand( CommandSender::orderType::DELETEPEER,peer );
    } catch( std::exception &e )
    {
        qDebug ()<<"problem while order execution"<<e.what();
    }
}

void MainWindow::onAddPeer( const QString& peer_str )
{
    try {
        MeshPeer peer;
        peer.setInvitation( peer_str );
        m_sender->sendCommand( CommandSender::orderType::ADDPEER,peer );
    } catch( std::exception& e ) {
        qDebug()<<"problem while order execution";
    }
}


void MainWindow::onSendMessage( const QString &vip, const QString &msg )
{
    MeshPeer peer;
    peer.setVip( vip );
    // to wiadomosc std string
    //! @todo wykonanie komendy msg
}

void MainWindow::onFindPeer( const QString &vip )
{
    //dobrac wlasciwa komende
}

void MainWindow::on_actionsettings_triggered()
{
    qDebug()<<"settings";
    hostDialog dlg;
    connect( &dlg,SIGNAL( host_info( QString,uint16_t ) ),this,SLOT( add_host_info( QString,uint16_t ) ) );
    dlg.exec();
//    addressDialog dialog;
//    dialog.exec();
}

void MainWindow::onPeerRemoved( const QString &vip )
{
    ui->peerListWidget_2->onPeerRemoved( vip );
}

void MainWindow::onPeerAdded( const QString &invitation )
{

    ui->peerListWidget_2->onPeerAdded( invitation );
}

//void MainWindow::onPeerBanned(const QString &vip)
//{
////	ui->peerListWidget_2->onPeerBanned()
//}


bool MainWindow::isWorking()
{
    bool value = m_status_form->isWorking();
//    ui->peerListWidget_2->setEnabled( value );
//    ui->quickStart->setEnabled( value );
    return value;
}

void MainWindow::onBanAll()
{
    m_sender->sendCommand( CommandSender::orderType::BANALL );
}

void MainWindow::onDeleteAll()
{
    m_sender->sendCommand( CommandSender::orderType::DELETEALL );
}

StatusForm* MainWindow::GetStatusObject()
{
    return m_status_form;
    ;
}

void MainWindow::onNetConnected( bool val )
{
    isWorking();
    if( !val ) {
        QErrorMessage message( this );
        message.showMessage( tr( "can't connect to node. check File ->settings" ) );
        message.exec();
    } else {
        QErrorMessage msg( this );
        msg.showMessage( "connected to node" );
        msg.exec();
    }
}


void MainWindow::onDeletePeer( QString &vip )
{
    ui->peerListWidget_2->onPeerRemoved(vip);
}
