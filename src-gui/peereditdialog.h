#ifndef PEEREDITDIALOG_H
#define PEEREDITDIALOG_H

#include <QDialog>

namespace Ui {
class PeerEditDialog;
}

class PeerEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PeerEditDialog(QWidget *parent = 0);
    ~PeerEditDialog();

    void setData(const QString &name, const QString &vip, const QString &ip);

    QString getName();
    QString getIp();
    QString getVip();
    QString getInvitation();

signals:
    void editPeer(QString);

protected:
    bool isIpv6(const QString &str);
    bool isIpv4(const QString &str);
private slots:
    void on_QrScanButton_clicked();

    void on_QrZoomButton_clicked();

    void on_VipEdit_editingFinished();

    void on_ipEdit_editingFinished();

    void on_buttonBox_accepted();

private:
    QString createInvitation();
    QString getPeerString();

    Ui::PeerEditDialog *ui;
};

#endif // PEEREDITDIALOG_H
