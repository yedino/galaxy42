#include <qdesktopservices.h>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>

#include "qrdialog.h"
#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"
#include <qrencode.h>

quickStartDialog::quickStartDialog( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::quickStartDialog )
{
    ui->setupUi( this );
    ui->InviteEdit->setText("fd42:6e4d:ce1e:c54a:ed78:0a4e:bbed:126e@176.31.171.15");
}

void quickStartDialog::setIps( const QString &ip,const QString &invitation )
{
    qDebug()<<"set invitations";
    Q_UNUSED(ip)
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
    QString page_address = "www.meshnet.pl";

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
	QRcode *qr = QRcode_encodeString(ui->myInviteEdit->text().toStdString().c_str(), 1, QR_ECLEVEL_L, QR_MODE_8, 1);
	if( !qr )
		throw std::runtime_error("Invalid QR generation!!!");

	QPixmap qr_code;
	unsigned char rect_size = 6;
	QImage tmp(qr->width*rect_size, qr->width*rect_size, QImage::Format::Format_RGB32);
	QColor color(Qt::white);
	tmp.fill(color);

	for(int yy = 0; yy < qr->width; yy++)
		for(int xx= 0; xx < qr->width; xx++)
			if( qr->data[yy*qr->width+xx] & 0x01 )
				for(int i = 0; i < rect_size; i++)
					for(int j = 0; j < rect_size; j++)
						tmp.setPixelColor(xx*rect_size+i, yy*rect_size+j, Qt::black);
	qr_code = QPixmap::fromImage(tmp);

	QRcode_free(qr);
//    ui->QrZoomButton->setIcon(); //setting new icons
    QrDialog dlg( qr_code );
    dlg.exec();
}

void quickStartDialog::on_whatIsVipBtn_clicked()
{
    QMessageBox box(QMessageBox::Information,"What's up doc tip","This is your virtual IP.");
    box.exec();
}

void quickStartDialog::on_whatisFullInviteBtn_clicked()
{

    QMessageBox box(QMessageBox::Information,"What's up doc tip","This is your full invite address.\n You can generete QR code with it or copy/paste it manualy.");
    box.exec();
}


void quickStartDialog::on_whatisPasteInv_clicked()
{

    QMessageBox box(QMessageBox::Information,"What's up doc tip","This is place to insert full invite from your friend.\n You scan it from QR code or set it manualy.");
    box.exec();
}
