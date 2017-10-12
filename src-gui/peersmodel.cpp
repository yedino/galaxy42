#include <QDebug>
#include <QColor>
#include <QIcon>
#include <QBrush>

#include "peersmodel.h"

peersModel::peersModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant peersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section){

    }

    if(orientation == Qt::Horizontal){
        if(role== Qt::DisplayRole){
            return QString ("peer list");
        }
    }
    return QVariant();
}

int peersModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_peers_list.size();
}

QVariant peersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole){

        switch (index.column()) {
        case name:
            return m_peers_list.at(index.row())->getName();
        case ip:
            return m_peers_list.at(index.row())->getIp();
        case vip:
            return m_peers_list.at(index.row())->getVip();
        case invitation:
          return m_peers_list.at(index.row())->getInvitation();
         default:
            return QVariant();
        break;
        }
    } else if(role == Qt::DecorationRole) {				//! @todo add icons for proper states
        switch (m_peers_list.at( index.row())->status) {
        case MeshPeer::defalut:
            return QIcon(":defalut");
        case MeshPeer::connected:
            return QIcon(":connecting");
//            return QColor(Qt::red);
            break;
        case MeshPeer::disconnected:
            return QIcon(":peer");
//            return QColor(Qt::green);
            break;
        case MeshPeer::banned:
            return QIcon(":banned");
//            return QColor(Qt::blue);
            break;
        case MeshPeer::bfrend:
                return QIcon(":peer");
//            return QColor(Qt::cyan);
            break;
        default:
            break;
        }
    }else if( role == Qt::ForegroundRole){
        switch (m_peers_list.at( index.row())->comm_status) {
        case MeshPeer::sended:
            return QBrush( QColor(Qt::yellow));
        case MeshPeer::recived:
            return QBrush (QColor(Qt::blue));
        case MeshPeer::acknowled:
            return QBrush(QColor(Qt::green));
        default:
            return QBrush(QColor(Qt::gray));
            break;
        }
    }
    return QVariant();
}

void peersModel::addPeer(QString serialized_peer)
{
    MeshPeer *peer;

    try {
        peer = new MeshPeer(serialized_peer,this);
        foreach (auto it, m_peers_list) {
            if(it->getVip() == peer->getVip()) {
                peer->status = MeshPeer::connected;
                peer->comm_status = MeshPeer::sended;
//               peer->comm_status = MeshPeer::acknowled;
                qDebug()<<"added peer";
                return;
            }
        }
        peer->status = MeshPeer::connected;
        peer->comm_status = MeshPeer::acknowled;
    } catch (std::runtime_error &e) {
        //! @todo dodac okno dialogowe bledu
        qDebug()<< "can't create peer::"<< QString(e.what());
        return;
    }
    auto idx = index(0,0);

    beginInsertRows(idx,0,1);
    m_peers_list.push_back(peer);
    endInsertRows();
}

void peersModel::addPeer(const MeshPeer &pp)
{
    MeshPeer *peer;
    try {
        peer = new MeshPeer(pp);
        foreach (auto it, m_peers_list) {
            if(it->getVip() == peer->getVip()) {
                peer->status = MeshPeer::connected;
//               peer->comm_status = MeshPeer::acknowled;
                qDebug()<<"added peer";
                return;
            }
        }
        peer->status = MeshPeer::connected;
        peer->comm_status = MeshPeer::acknowled;

        QModelIndex ix = index(m_peers_list.size());
        beginInsertRows(ix,m_peers_list.size(),m_peers_list.size()+1);
        m_peers_list.push_back(peer);
        endInsertRows();
    } catch (std::runtime_error &e) {
        //! @todo dodac okno dialogowe bledu
        qDebug()<< "can't create peer::"<< QString(e.what());
        return;
    }
}

void peersModel::removePeer(const QModelIndex& index)
{
    if(!index.isValid()){
        qDebug()<<"can't remove from invalid index:"<<index;
        return;
    }

    beginRemoveRows(index,index.row(),index.row());
    m_peers_list.removeAt(index.row());
    endRemoveRows();
}

void peersModel::confirmAddPeer(const MeshPeer& peer)
{
    try{
        auto pp = findPeer(peer);
        pp->status = MeshPeer::connected;
        pp->comm_status = MeshPeer::acknowled;

    } catch(std::runtime_error &e) {
        qDebug()<<e.what();
        beginInsertRows(createIndex(m_peers_list.size(),0),m_peers_list.size(),m_peers_list.size()+1);
        MeshPeer *pp = new MeshPeer(peer);
        pp->status = MeshPeer::connected;
        pp->comm_status = MeshPeer::acknowled;
        pp->setName("quick add");
        m_peers_list.push_back(pp);
        endInsertRows();
    }
}

void peersModel::confirmDeletePeer(const MeshPeer& peer)
{
    try{
//        auto pp = findPeer(peer);
        size_t i = 0;
        foreach (auto it, m_peers_list) {
            if(it->getVip() == peer.getVip()){
                beginRemoveRows(index(i),i,i+1);
                m_peers_list.removeAt(i);
                endRemoveRows();
                return;
            };
            i++;
        }
    }catch(std::runtime_error &e){
        qDebug()<<e.what();
    }

}

void peersModel::confirmBanPeer(const MeshPeer & peer)
{
    try{
        auto pp = findPeer(peer);
        pp->status = MeshPeer::banned;
        pp->comm_status = MeshPeer::recived;

    } catch(std::runtime_error &e) {
        qDebug()<<e.what();
    }
}

void peersModel::updatePeerList(QList<MeshPeer*> new_list)
{
    deleteOldList();

    foreach (auto it, new_list) {
        it->comm_status = MeshPeer::acknowled;
    }

    beginInsertRows(createIndex(0,0),0,new_list.size());
    m_peers_list = new_list;
    endInsertRows();
}


MeshPeer* peersModel::findPeer(const MeshPeer& peer)
{
    foreach (auto it, m_peers_list) {
        if(it->getVip() == peer.getVip()) return it;
    }
    throw std::runtime_error ("can't find peer");
}


void  peersModel::deleteOldList()
{
    //! @todo dopisac usuniecie starej listy
    beginRemoveRows(index(0),0,m_peers_list.size());
    foreach (auto it, m_peers_list) {
        delete it;
    }
    m_peers_list.clear();
    endRemoveRows();
}

MeshPeer* peersModel::findPeer(QString value,Columns col)
{
    MeshPeer *peer;
    for(int i=0; i < m_peers_list.size() ; ++i) {
        if(data(index(i,col)).toString() == value) return m_peers_list.at(i);
    }

    throw std::runtime_error ("can't find peer");
}

QModelIndex peersModel::findIndex(const QString& value, Columns col)
{
    for (int i=0 ; i < m_peers_list.size();++i ){
        QModelIndex idx = index(i,col);
        if(data(idx).toString() == value) return idx;
    }
    return QModelIndex();
}
