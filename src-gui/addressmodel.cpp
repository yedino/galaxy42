#include "addressmodel.h"

addressModel::addressModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant addressModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if(role == Qt::DisplayRole) {
        switch (static_cast< ColumnRoles>(section)) {

        case ColumnRoles::state:
            return tr("State");
            break;

        case ColumnRoles::name:
            return tr("Name");
            break;

        case ColumnRoles::ip:
            return tr("IP");
            break;

        case ColumnRoles::vip:
            return tr("VIP");
            break;

        case ColumnRoles::port:
            return tr("Port");
            break;

        case ColumnRoles::vip_port:
            return tr("virtual Port");
            break;

        case ColumnRoles::source:
            return tr("Source");
            break;

        case ColumnRoles::comm_status:
            return tr("Command status");
            break;

        default:
            break;
        }
    }else {//! @todo implement some to nice look to decoration role
        return QVariant();
    }
    return QVariant();

    // FIXME: Implement me!
}

bool addressModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex addressModel::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!

    return QModelIndex();
}


QModelIndex addressModel::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
    return QModelIndex();
}

int addressModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return m_peers.size();
}

int addressModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;
    return m_enabled_columns.size();

}

bool addressModel::hasChildren(const QModelIndex &parent) const
{
    return false;
    // FIXME: Implement me!
}

bool addressModel::canFetchMore(const QModelIndex &parent) const
{
    // FIXME: Implement me!
    return false;
}

void addressModel::fetchMore(const QModelIndex &parent)
{
    // FIXME: Implement me!
}

QVariant addressModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
            return getData(index);
        break;
        case Qt::DecorationRole:
            return getDecorator(index);
        break;
        case Qt::EditRole:
            return getEditor(index);
        break;
        default:
        break;
    }


/*
    if(role == Qt::DisplayRole){
        switch (static_cast<ColumnRoles>(index.column())) {
        case ColumnRoles::state:
            return m_peers.at(index)->status;//! status ewentualnie przeksztalcenie z inta na jakas string
            break;
        case ColumnRoles::name:
            return m_peers.at(index)->getName();
            break;
        case ColumnRoles::ip:
            return m_peers.at(index)->getIp();
            break;

        case ColumnRoles::port:
            return m_peers.at(index)->getPort
   ;
            break;
    default:
        break;
    }
    }
*/
    // FIXME: Implement me!
    return QVariant();
}

QVariant addressModel::getData(const QModelIndex &index) const
{
        switch (static_cast< ColumnRoles>(index.column())) {

        case ColumnRoles::state:
            return static_cast<int>(m_peers.at(index.row())->status);
            break;

        case ColumnRoles::name:
            return m_peers.at(index.row())->getName();
            break;

        case ColumnRoles::ip:
            return m_peers.at(index.row())->getIp();
            break;

        case ColumnRoles::vip:
            return m_peers.at(index.row())->getVip();
            break;

        case ColumnRoles::port:
            return m_peers.at(index.row())->getPort();
            break;

        case ColumnRoles::vip_port:
            return m_peers.at(index.row())->getVipPort();
            break;

        case ColumnRoles::source:
//            return m_peers.at(index.row())->g;
            return QVariant();
            break;

        case ColumnRoles::comm_status:
            return static_cast<int>(m_peers.at(index.row())->status);
            break;
    default:
        break;
    }
    return QVariant();
}

QVariant addressModel::getDecorator(const QModelIndex &index) const
{
    switch( static_cast< ColumnRoles>(index.column()) ) {
    case ColumnRoles::ip:

        break;
    default:
        break;
    }
    return QVariant();
}

QVariant addressModel::getEditor(const QModelIndex &index) const
{

    /**
      create editor window
    */
    return QVariant();
}


void addressModel::setValue(ColumnRoles role,int row,const QVariant& value)
{
    MeshPeer *working_peer = m_peers.at(row);

    switch (role) {
    case ColumnRoles::ip:
        working_peer->setIP(value.toString());
        break;
   case ColumnRoles::port:
        working_peer->setPort(value.toInt());
        break;
    case ColumnRoles::name:
        working_peer->setName(value.toString());
        break;
    case ColumnRoles::vip_port:
        working_peer->setVipPort(value.toInt());
        break;
    case ColumnRoles::vip:
        working_peer->setName(value.toString());
        break;
    case ColumnRoles::comm_status:				//can't edit from list - may be some triger to action?
        break;
    case ColumnRoles::source:					//can't edit this
        break;
    default:
        break;
    }
}

bool addressModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        setValue(static_cast < ColumnRoles>(index.column()), index.row(),value);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags addressModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    ColumnRoles role = static_cast<ColumnRoles>(index.column());

    if(role == ColumnRoles::source || role == ColumnRoles::source || role == ColumnRoles::comm_status){
        return Qt::ItemIsEnabled;			//!@todo find proper flags
    } else {
        return Qt::ItemIsEditable; // FIXME: Implement me!
    }
}

bool addressModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return false;
}

bool addressModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return false;
}

bool addressModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    for(int i = 0;i<count;++i) {
        if(m_peers.size()< row + count -i ){
            endRemoveRows();
            return true;
        }
        m_peers.removeAt( row + count - i );
    }

    endRemoveRows();
    return false;
}

bool addressModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return false;
}

void addressModel::addPeer(MeshPeer* peer)
{

//    beginInsertRows();
    m_peers.push_back(peer);
//    endInsertRows();

}

void addressModel::addPeers(QList<MeshPeer*> peers)
{
//    beginInsertRows(index(m_peers.size(),0));
    for(auto it :peers) {
        m_peers.push_back(it);
    }
//    endInsertRows();
}
