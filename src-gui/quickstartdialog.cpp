#include <qdesktopservices.h>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>

#include "qrdialog.h"
#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"

quickStartDialog::quickStartDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::quickStartDialog)
{
   ui->setupUi(this);
}

void quickStartDialog::setIps(const QString &ip,const QString &invitation)
{
    qDebug()<<"set invitations";

    QString vip = IpReader::readVipFromInvitation(invitation);
    ui->vipEdit->setText(vip);
    ui->myInviteEdit->setText(invitation);

//   ui->vipEdit->setText(vip);
//   QString invite = QString (vip+"-"+ip);
//   ui->myInviteEdit->setText(vip);

//   QStringList <QString> adresses = vip.split('@');
//   QStringList <QString> ipv4adresses;
//   QString virtualIp;
}


quickStartDialog::~quickStartDialog()
{
    delete ui;
}

void quickStartDialog::chooseInvate(QString invate)
{
    ui->InviteEdit->setText(invate);		//@todo dodac sprawdzenie czy invate jest prawidlowe
}

void quickStartDialog::on_allowFriendsBox_clicked(bool checked)
{
    emit(allowFriend(checked));
}


void quickStartDialog::on_allowStrangersBox_clicked(bool checked)
{
    emit(allowStranger(checked));
}

void quickStartDialog::on_wwwButton_clicked()
{
    QString page_address = "www.google.com";

    QUrl url(page_address);
    bool done =	QDesktopServices::openUrl(url);
    if(!done) {
        QMessageBox msg(QMessageBox::Warning ,QString("browser problem"),QString("can't open browser"));
    }
}

void quickStartDialog::on_connectButton_clicked()
{
    if(ui->InviteEdit->text().isEmpty()) {
        emit createNet();
    } else {
        QString ip = ui->InviteEdit->text();
        ip = ip.split('-').at(0);

        emit connectNet(ip);		//!@todo dorobic sprawdzenie czy wprowadzony tekst jest poprawny
    }
}

void quickStartDialog::on_QrScanButton_clicked()
{

}

void quickStartDialog::on_QrZoomButton_clicked()
{
    QPixmap qr_code;			//@todo wstawic zeczywisty qrcode
    QrDialog dlg(qr_code);
    dlg.exec();
}
