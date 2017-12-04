#ifndef PEERSMODEL_H
#define PEERSMODEL_H

#include <QAbstractListModel>
#include "meshpeer.h"

class peersModel : public QAbstractListModel
{
    Q_OBJECT
    QList<MeshPeer*> m_peers_list;

public:
    enum class Columns{
        name = 0,
        ip,
        vip,
        invitation,
        status,
        icon
    };

    explicit peersModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        parent.column();
        return 4;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex findIndex(const QString &value, Columns col);

    bool peerExist(const QString &vip);
public slots:
    void addPeer(QString serialized_peer);
    void addPeer(const MeshPeer &peer);
    void removePeer(const QModelIndex& index);
    void confirmAddPeer(const MeshPeer& peer);
    void confirmDeletePeer(const MeshPeer& peer);
    void confirmBanPeer(const MeshPeer & peer);
    void updatePeerList(QList<MeshPeer*> new_list);
    void deletaAllPeers();
    void banAllPeers();
    void startActionOnIndex(const QModelIndex &index);
//    void r(const QModelIndex& index);

    MeshPeer* findPeer(QString value,Columns col);

signals:

    void deletePeerSignal(const QString& invitation);
    void addPeerSignal(const QString &invitation);
    void banPeerSignal(const QString &inbitation);
    void pingPeer(const QString &vip);

private:
    void deleteOldList();
    MeshPeer* findPeer(const MeshPeer& peer);


};

#endif // PEERSMODEL_H
