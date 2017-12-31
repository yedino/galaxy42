#include <QDebug>
#include <QColor>
#include <QIcon>
#include <QBrush>
#include <QMessageBox>

#include "peersmodel.h"

peersModel::peersModel( QObject *parent )
    : QAbstractListModel( parent )
{
}

QVariant peersModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section ) {

    }

    if( orientation == Qt::Horizontal ) {
        if( role== Qt::DisplayRole ) {
            return QString ( "peer list" );
        }
    }
    return QVariant();
}

int peersModel::rowCount( const QModelIndex &parent ) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if ( parent.isValid() )
        return 0;

    return m_peers_list.size();
}

QVariant peersModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if( role == Qt::DisplayRole ) {

        switch ( static_cast <Columns>(index.column()) ) {
        case Columns::name:
            return m_peers_list.at( index.row() )->getName();
        case Columns::ip:
            return m_peers_list.at( index.row() )->getIp();
        case Columns::vip:
            return m_peers_list.at( index.row() )->getVip();
        case Columns::invitation:
            return m_peers_list.at( index.row() )->getInvitation();
        case Columns::status:
            return static_cast<int>(m_peers_list.at(index.row() )->status);
        default:
            return QVariant();
            break;
        }
    } else if( role == Qt::DecorationRole ) {				//! @todo add icons for proper states
        if(m_peers_list.at(index.row())->comm_status == MeshPeer::COMMANDSTATUS::sended) {
            return QIcon(":status-busy");
        }

        switch ( m_peers_list.at( index.row() )->status ) {
        case MeshPeer::STATUS::defalut:
            return QIcon( ":status-offlline" );
        case MeshPeer::STATUS::connected:
            return QIcon( ":status-online" );
//            return QColor(Qt::red);
            break;
        case MeshPeer::STATUS::disconnected:
            return QIcon( ":disconnect" );
//            return QColor(Qt::green);
            break;
        case MeshPeer::STATUS::banned:
            return QIcon( ":ladybird" );
//            return QColor(Qt::blue);
            break;
        case MeshPeer::STATUS::bfrend:
            return QIcon( ":status-online" );
//            return QColor(Qt::cyan);
            break;
        default:
            break;
        }
    } else if( role == Qt::ForegroundRole ) {
        switch ( m_peers_list.at( index.row() )->comm_status ) {
        case MeshPeer::COMMANDSTATUS::sended:
            return QBrush( QColor( Qt::darkYellow ) );
        case MeshPeer::COMMANDSTATUS::recived:
            return QBrush ( QColor( Qt::blue ) );
        case MeshPeer::COMMANDSTATUS::acknowled:
            return QBrush( QColor( Qt::darkGreen ) );
        default:
            return QBrush( QColor( Qt::gray ) );
            break;
        }
    }
    return QVariant();
}


bool peersModel::peerExist(const QString &vip)
{
    for (auto it :m_peers_list){
        if(it->getVip() == vip) return true;
    }
    return false;
}

void peersModel::addPeer( QString serialized_peer )
{
    MeshPeer *peer;

    try {
        peer = new MeshPeer( serialized_peer,this );
        for ( auto it: m_peers_list ) {
            if( it->getVip() == peer->getVip() ) {
                peer->status = MeshPeer::STATUS::connected;
                peer->comm_status = MeshPeer::COMMANDSTATUS::sended;
//               peer->comm_status = MeshPeer::acknowled;
//                qDebug()<<"added peer";
                QMessageBox box(QMessageBox::Icon::Warning,"Peer add","Peer already exist");
                box.exec();
                return;
            }
        }
        peer->status = MeshPeer::STATUS::connected;
        peer->comm_status = MeshPeer::COMMANDSTATUS::sended;
    } catch ( std::runtime_error &e ) {
        //! @todo dodac okno dialogowe bledu
        qDebug()<< "can't create peer::"<< QString( e.what() );
        return;
    }
    auto idx = index( 0,0 );

    beginInsertRows( idx,0,1 );
    m_peers_list.push_back( peer );
    endInsertRows();
}

void peersModel::addPeer( const MeshPeer &pp )
{
    MeshPeer *peer;
    try {
        peer = new MeshPeer( pp );
        for ( auto it: m_peers_list ) {
            if( it->getVip() == peer->getVip() ) {
                peer->status = MeshPeer::STATUS::connected;
//               peer->comm_status = MeshPeer::acknowled;
                qDebug()<<"added peer";
                return;
            }
        }
//        peer->status = MeshPeer::connected;
//        peer->comm_status = MeshPeer::acknowled;

        QModelIndex ix = index( m_peers_list.size() );
        beginInsertRows( ix,m_peers_list.size(),m_peers_list.size()+1 );
        m_peers_list.push_back( peer );
        endInsertRows();
    } catch ( std::runtime_error &e ) {
        //! @todo dodac okno dialogowe bledu
        qDebug()<< "can't create peer::"<< QString( e.what() );
        return;
    }
}

void peersModel::removePeer( const QModelIndex& index )
{
    if( !index.isValid() ) {
        qDebug()<<"can't remove from invalid index:"<<index;
        return;
    }

    beginRemoveRows( index,index.row(),index.row() );
    m_peers_list.removeAt( index.row() );
    endRemoveRows();
}

void peersModel::confirmAddPeer( const MeshPeer& peer )
{
    try {
        auto pp = findPeer( peer );
        pp->status = MeshPeer::STATUS::connected;
        pp->comm_status = MeshPeer::COMMANDSTATUS::acknowled;
        emit dataChanged(index(0,0),index(m_peers_list.size()-1,0));									//!@todo check the minimal size of change
    } catch( std::runtime_error &e ) {
        qDebug()<<e.what();
        beginInsertRows( createIndex( m_peers_list.size(),0 ),m_peers_list.size(),m_peers_list.size()+1 );
        MeshPeer *pp = new MeshPeer( peer );
        pp->status = MeshPeer::STATUS::connected;
        pp->comm_status = MeshPeer::COMMANDSTATUS::acknowled;
        pp->setName( "quick add" );
        m_peers_list.push_back( pp );
        endInsertRows();
    }
}

void peersModel::confirmDeletePeer( const MeshPeer& peer )
{
    try {
//        auto pp = findPeer(peer);
        size_t i = 0;
        foreach ( auto it, m_peers_list ) {
            if( it->getVip() == peer.getVip() ) {
                beginRemoveRows( index( i ),i,i+1 );
                m_peers_list.removeAt( i );
                endRemoveRows();
                return;
            };
            i++;
        }
    } catch( std::runtime_error &e ) {
        qDebug()<<e.what();
    }

}

void peersModel::confirmBanPeer( const MeshPeer & peer )
{
    try {
        auto pp = findPeer( peer );
        pp->status = MeshPeer::STATUS::banned;
        pp->comm_status = MeshPeer::COMMANDSTATUS::recived;

    } catch( std::runtime_error &e ) {
        qDebug()<<e.what();
    }
}

void peersModel::updatePeerList( QList<MeshPeer*> new_list )
{
    deleteOldList();

    foreach ( auto it, new_list ) {
        it->comm_status = MeshPeer::COMMANDSTATUS::acknowled;
    }

    beginInsertRows( createIndex( 0,0 ),0,new_list.size()-1 );
    m_peers_list = new_list;
    endInsertRows();
}


MeshPeer* peersModel::findPeer( const MeshPeer& peer )
{
    foreach ( auto it, m_peers_list ) {
        if( it->getVip() == peer.getVip() ) return it;
    }
    throw std::runtime_error ( tr( "can't find peer" ).toStdString() );
    return nullptr;
}


void  peersModel::deleteOldList()
{
    //! @todo dopisac usuniecie starej listy
    beginRemoveRows( index( 0 ),0,m_peers_list.size() );
    foreach ( auto it, m_peers_list ) {
        delete it;
    }
    m_peers_list.clear();
    endRemoveRows();
}

MeshPeer* peersModel::findPeer( QString value,Columns col )
{
    for( int i=0; i < m_peers_list.size() ; ++i ) {
        if( data( index( i,static_cast <int >( col) ) ).toString() == value ) return m_peers_list.at( i );
    }

    throw std::runtime_error ( tr( "can't find peer" ).toStdString() );
    return nullptr;
}

QModelIndex peersModel::findIndex( const QString& value, Columns col )
{
    for ( int i=0 ; i < m_peers_list.size(); ++i ) {
        QModelIndex idx = index( i,static_cast <int >( col) );
        if( data( idx ).toString() == value ) return idx;
    }
    return QModelIndex();
}


void peersModel::deletaAllPeers()
{
    if(m_peers_list.size() == 0){
        qDebug()<<"nothing to remove";
        return;
    }

    beginRemoveRows( index( 0 ),0,m_peers_list.size()-1 );
    foreach ( auto it, m_peers_list ) {
        delete it;
    }
    m_peers_list.clear();
    endRemoveRows();
}

void peersModel::banAllPeers()
{
    foreach ( auto it, m_peers_list ) {
        it->status = MeshPeer::STATUS::banned;
    }
}


void  peersModel::startActionOnIndex(const QModelIndex &index)
{
    try {
        size_t i = index.row();
        m_peers_list.at(i)->comm_status = MeshPeer::COMMANDSTATUS::sended;
    } catch(std::exception &e) {
        qDebug()<<e.what();
    }
}
