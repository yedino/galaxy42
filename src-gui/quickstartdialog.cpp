#include <qdesktopservices.h>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>

#include "qrdialog.h"
#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"

quickStartDialog::quickStartDialog( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::quickStartDialog )
{
    ui->setupUi( this );
    ui->InviteEdit->setText("fd42:6e4d:ce1e:c54a:ed78:0a4e:bbed:126e@176.31.171.15 ");
}

void quickStartDialog::setIps( const QString &ip,const QString &invitation )
{
    qDebug()<<"set invitations";

    QString vip = IpReader::readVipFromInvitation( invitation );
    ui->vipEdit->setText( vip );
    ui->myInviteEdit->setText( invitation );

}


quickStartDialog::~quickStartDialog()
{
    delete ui;
}

void quickStartDialog::chooseInvate( QString invate )
{
    ui->InviteEdit->setText( invate );		//!@todo check if invitation is proper
}

void quickStartDialog::on_allowFriendsBox_clicked( bool checked )
{
    emit( allowFriend( checked ) );
}


void quickStartDialog::on_allowStrangersBox_clicked( bool checked )
{
    emit( allowStranger( checked ) );
}

void quickStartDialog::on_wwwButton_clicked()
{
    QString page_address = "www.google.com";

    QUrl url( page_address );
    bool done =	QDesktopServices::openUrl( url );
    if( !done ) {
        QMessageBox msg( QMessageBox::Warning,QString( tr( "browser problem" ) ),QString( tr( "can't open browser" ) ) );
        msg.exec();
    }
}

void quickStartDialog::on_connectButton_clicked()
{
    if( ui->InviteEdit->text().isEmpty() ) {
        emit createNet();
    } else {
        QString ip = ui->InviteEdit->text();
        ip = ip.split( '-' ).at( 0 );

        emit connectNet( ip );		//!@todo check if ip is proper
    }
}

void quickStartDialog::on_QrScanButton_clicked()
{

}

void quickStartDialog::on_QrZoomButton_clicked()
{
    QPixmap qr_code;			//!@todo add real qrCode
//    ui->QrZoomButton->setIcon(); //setting new icons
    QrDialog dlg( qr_code );
    dlg.exec();
}
