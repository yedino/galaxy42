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
            return tr("Virtual Port");
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

    // FIXME: Implement me!
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

bool addressModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags addressModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable; // FIXME: Implement me!
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
    m_peers.removeAt(row);
    // FIXME: Implement me!
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
