#ifndef QUICKSTARTDIALOG_H
#define QUICKSTARTDIALOG_H

#include <QWidget>

namespace Ui {
class quickStartDialog;
}

class IpReader
{
public:
    IpReader(){;}

    //    "{\"cmd\":\"get_galaxy_new_format_reference\",\"msg\":\"fd42:957b:41b3:8570:b270:ef27:627e:914c@(udp:127.0.0.1:9042)\"}"


    static QStringList readIpFromInvitation(const QString &invitation)
    {
        QString splited = invitation.split('@').at(1);

        QStringList splited_ip = splited.split(":");

        QString  ip = splited.split(':').at(1)+":"+splited.split(':').at(2);
        return ip.split('@');
    }

    static QString readVipFromInvitation(const QString &invitation)
    {
        QString splited = invitation.split('@').at(0);
       return splited;
    }

};


class quickStartDialog : public QWidget
{
    Q_OBJECT

public:
    explicit quickStartDialog(QWidget *parent = 0);
    ~quickStartDialog();

    void setIps(const QString &ip,const QString &vip);
signals:

    void allowFriend(bool);				//! signal that allow firend
    void allowStranger(bool);			//!sygnal that allow strangers to net
    void createNet();					//! sygnal create net
    void connectNet(QString net);		//!sygnal connect net
    void addUser(QString user_id);		//!sygnal that add user
    void removeUser(QString user_id);	//!sygnal remove usera


public slots:
    void chooseInvate(QString invate);	//! slot from chose invittaion from address book

private slots:

    void on_allowFriendsBox_clicked(bool checked);		//! slot  of signal allow friend to connect
    void on_allowStrangersBox_clicked(bool checked);	//! slot check boxa stranger to connect


    void on_wwwButton_clicked();

    void on_connectButton_clicked();

    void on_QrScanButton_clicked();

    void on_QrZoomButton_clicked();

private:
    Ui::quickStartDialog *ui;
};

#endif // QUICKSTARTDIALOG_H
