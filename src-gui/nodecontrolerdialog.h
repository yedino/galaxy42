#ifndef NODECONTROLERDIALOG_H
#define NODECONTROLERDIALOG_H

#include <QDialog>

#include "commandsender.h"
#include "commandexecutor.hpp"

namespace Ui {
class NodeControlerDialog;
}

class NodeControlerDialog : public QDialog
{
    Q_OBJECT

public:

    explicit NodeControlerDialog(QWidget *parent = 0);
    explicit NodeControlerDialog(MainWindow *mw,QHostAddress address ,uint port);
    ~NodeControlerDialog();

    void onNewPeerList(const QStringList &peerList);

    void confirmAddPeer(const QString &vip);
    void confirmRemovePeer(const QString& vip);
    void confirmBanPeer(const QString &vip);

public slots:
    void onAddPeer(QString);
    void onRemovePeer(QString);
    void onBanPeer(QString);


private:
    Ui::NodeControlerDialog *ui;

    CommandSender* m_sender;
    commandExecutor *m_executor;

};

#endif // NODECONTROLERDIALOG_H
