#include <QMenu>
#include <QModelIndex>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMessageBox>

#include "peerlistform.h"
#include "ui_peerlistform.h"
#include "peereditdialog.h"
#include "mainwindow.hpp"

PeerListForm::PeerListForm( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::PeerListForm )
{
    ui->setupUi( this );
    peersModel * model = new peersModel( this );
    ui->listView->setModel( model );
    model->sort( peersModel::name );
    m_model = model;


}

PeerListForm::~PeerListForm()
{
    delete ui;
}

void PeerListForm::contextMenuEvent( QContextMenuEvent *event )
{
    if( m_main_window ) {
        if( ! m_main_window->isWorking() )
//        QMessageBox box(" no working connection");
            //       box.exec();
            return;
    }

    m_index = QModelIndex();
    if ( ui->listView->currentIndex().isValid() ) {
        m_index = ui->listView->currentIndex();
    }

    QPoint global_pos = event->globalPos();
    QAction* add_action = new QAction(QIcon(":add"), tr( "add" ),ui->listView );
    QAction* ban_action = new QAction(QIcon(":ladybird"), tr( "ban" ),ui->listView );
    QAction* ban_all_action  =new QAction(QIcon(":ladybird"), tr("ban all" ),ui->listView );
    QAction* remove_action = new QAction( QIcon(":delete"), tr("remove" ),ui->listView );
    QAction *remove_all_action = new QAction(QIcon(":remove"), tr( "remove all" ),ui->listView );
    QAction* send_massage = new QAction( tr( "send msg" ),ui->listView );
    QAction* ping_action = new QAction( tr( "ping" ),ui->listView );
    QAction* find_action = new QAction( tr( "find" ),ui->listView );

    connect( add_action, SIGNAL( triggered( bool ) ), this, SLOT( addActionSlot( bool ) ) );
    connect( ban_action,SIGNAL( triggered( bool ) ), this, SLOT( banActionSlot( bool ) ) );
    connect( remove_action,SIGNAL( triggered( bool ) ), this, SLOT( removeActionSlot( bool ) ) );
    connect( send_massage,SIGNAL( triggered( bool ) ), this, SLOT( sendMessageActionSlot( bool ) ) );
    connect( ping_action,SIGNAL( triggered( bool ) ), this, SLOT( pingActionSlot( bool ) ) );
    connect( find_action,SIGNAL( triggered( bool ) ),this,SLOT( findActionSlot( bool ) ) );
    connect( ban_all_action,SIGNAL( triggered( bool ) ),this,SLOT( banAllSlot( bool ) ) );
    connect( remove_all_action,SIGNAL( triggered( bool ) ),this,SLOT( deleteAllSlot( bool ) ) );

    QMenu menu;

    menu.addAction( add_action );
    if( m_index.isValid() ) {
        menu.addAction( ban_action );
        menu.addAction( remove_action );
        menu.addAction( send_massage );
    }
    menu.addAction( find_action );
    menu.addAction( ban_all_action );
    menu.addAction( remove_all_action );
    menu.exec( global_pos );

}

void PeerListForm::sendMessageActionSlot( bool )
{
    qDebug()<<"send message";
}

void PeerListForm::addActionSlot( bool )
{
    qDebug()<<"add peer";

    if( m_index.isValid() ) {
        QString vip = m_model->data( m_index.sibling( m_index.row(),peersModel::invitation ) ).toString();
        emit ( addPeer( vip ) );
    } else {
        PeerEditDialog dlg;
        dlg.exec();
        QString invitation = dlg.getInvitation();
        if( invitation.size()== 0 ) {
            return;
        }

        MeshPeer peer( this );
        peer.setName( dlg.getName() );
        peer.setIP( dlg.getIp() );
        peer.setVip( dlg.getVip() );
        peer.comm_status = MeshPeer::COMMANDSTATUS::sended;
        m_model->addPeer( QString::fromStdString( peer.serialize() ) );
        emit ( addPeer( invitation ) );
    }
}

void PeerListForm::banActionSlot( bool )
{
    qDebug()<<"ban peer";
    QString vip = m_model->data( m_index.sibling( m_index.row(),peersModel::vip ) ).toString();
    m_model->startActionOnIndex(m_index);
    emit ( banPeer( vip ) );
}

void PeerListForm::removeActionSlot( bool )
{
    qDebug()<<"remove peer";
    if( m_index.isValid() ) {

        QModelIndex work_index = m_index.sibling( m_index.row(),peersModel::vip );
        QString vip = m_model->data( work_index ).toString();
        m_model->startActionOnIndex(work_index);
        emit ( removePeer( vip ) );
//        onPeerRemoved( vip );
    }
}

void PeerListForm::pingActionSlot( bool )
{
    qDebug()<<"ping peer";

    QString vip = m_model->data( m_index.sibling( m_index.row(),peersModel::vip ) ).toString();
    emit ( pingPeer( vip ) );

}

peersModel* PeerListForm::getModel()
{
    return m_model;
}

void PeerListForm::findActionSlot( bool )
{
    QString vip = m_model->data( m_index.sibling( m_index.row(),peersModel::vip ) ).toString();
    emit( findPeer( vip ) );
}

void PeerListForm::onPeerFounded( QString invitation,QString vip )
{
    MeshPeer* peer = m_model->findPeer( vip,peersModel::vip );

    peer->setInvitation( invitation );
    peer->status = MeshPeer::STATUS::connected;
}

void PeerListForm::onPeerBanned( const QString &vip )
{
    try {
        MeshPeer peer;
        peer.setVip( vip );
        m_model->confirmBanPeer( peer );
    } catch( std::exception &e ) {
        qDebug()<<e.what();
    }
}

void PeerListForm::onPeerAdded( const QString &invitation )
{
    try {
        MeshPeer peer;
        peer.setInvitation( invitation );
        m_model->confirmAddPeer( peer );
    } catch ( std::exception &e ) {
        qDebug()<<e.what();
    }
}

void PeerListForm::onPeerRemoved( const QString &vip )
{
    try {
        MeshPeer peer;
        peer.setVip( vip );
        m_model->confirmDeletePeer( peer );
    } catch( std::exception &e ) {
        qDebug()<<e.what();
    }
}

void PeerListForm::addPeer( const MeshPeer & peer )
{
//	m_model->addPeer();
}



void PeerListForm::banAllSlot( bool )
{
//    m_model->bannAll();
    emit banAll();
}

void PeerListForm::deleteAllSlot( bool )
{
//    m_model->removeAll();
    emit deleteAll();
}

void PeerListForm::on_pushButton_clicked()
{
    addActionSlot(true);
}



void PeerListForm::on_pushButton_2_clicked()
{
    removeActionSlot(true);
}
