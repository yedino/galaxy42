#ifndef ADDRESSMODEL_H
#define ADDRESSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "meshpeer.h"


class addressModel : public QAbstractItemModel
{
    Q_OBJECT

public:

   enum class ColumnRoles {
        state = 0,
        name,
        ip,
        port,
        vip,
        vip_port,
       source,
       comm_status
    };


    explicit addressModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void addPeer(MeshPeer* peer);
    void addPeers(QList<MeshPeer*> peers);

private:

    QVariant getData(const QModelIndex& index) const;
    QVariant getDecorator(const QModelIndex& index) const;
    QVariant getEditor(const QModelIndex& index) const;

    void setValue(ColumnRoles role,int row,const QVariant& value);

    QList<MeshPeer*> m_peers;
    QList<ColumnRoles> m_enabled_columns;

};

#endif // ADDRESSMODEL_H
