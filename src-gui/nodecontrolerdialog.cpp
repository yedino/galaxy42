#include "nodecontrolerdialog.h"
#include "ui_nodecontrolerdialog.h"
#include "meshpeer.h"
#include <QHostAddress>

NodeControlerDialog::NodeControlerDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::NodeControlerDialog )
{

    ui->setupUi( this );
}

NodeControlerDialog::NodeControlerDialog( MainWindow* mw,QHostAddress address, uint port )
{
    ui->setupUi( this );

    connect( ui->peerListWidget,SIGNAL( addPeer( QString ) ),this,SLOT( onAddPeer( QString ) ) );
    connect( ui->peerListWidget,SIGNAL( removePeer( QString ) ),this,SLOT( onRemovePeer( QString ) ) );
    connect( ui->peerListWidget,SIGNAL( banPeer( QString ) ),this,SLOT( onBanBeer( QString ) ) );

    m_executor = new commandExecutor( mw ) ; //commandExecutor::construct(std::make_shared<MainWindow>(this));//std::make_shared<commandExecutor>(new commandExecutor(this));	//commandExecutor::construct(ret);
    m_sender = new CommandSender( m_executor,mw );
    m_executor->setSender( m_sender );
    QString host;

    m_executor->startConnect( QHostAddress( address ), port );		//! @todo think about last send
}

NodeControlerDialog::~NodeControlerDialog()
{
    delete ui;
}

void NodeControlerDialog::confirmAddPeer( const QString& vip )
{

}

void NodeControlerDialog::confirmRemovePeer( const QString& vip )
{

}

void NodeControlerDialog::confirmBanPeer( const QString &vip )
{

}

void NodeControlerDialog::onAddPeer( QString peer_str )
{
    try {
        MeshPeer peer;
        peer.setInvitation( peer_str );
        m_sender->sendCommand( CommandSender::orderType::ADDPEER,peer );
    } catch( std::exception& e ) {
        qDebug()<<"problem while order execution";
    }

}

void NodeControlerDialog::onRemovePeer( QString peer_str )
{
    try {
        MeshPeer peer;
        peer.setInvitation( peer_str );
        m_sender->sendCommand( CommandSender::orderType::DELETEPEER,peer );
    } catch( std::exception& e ) {
        qDebug()<<"problem while order execution";
    }

}

void NodeControlerDialog::onBanPeer( QString peer_str )
{
    try {
        MeshPeer peer;
        peer.setInvitation( peer_str );
        m_sender->sendCommand( CommandSender::orderType::BANPEER,peer );
    } catch( std::exception& e ) {
        qDebug()<<"problem while order execution";
    }

}

void NodeControlerDialog::onNewPeerList( const QStringList &peerList )
{

}

