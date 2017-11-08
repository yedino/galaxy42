#include <QHostAddress>

#include "meshpeer.h"
#include "peereditdialog.h"
#include "ui_peereditdialog.h"


PeerEditDialog::PeerEditDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::PeerEditDialog )
{
    ui->setupUi( this );
    ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( false );
}


QString PeerEditDialog::getPeerString()
{
    MeshPeer peer;
    try {
        peer.setName( ui->nameEdit->text() );
        peer.setIP( ui->ipEdit->text() );
        peer.setVip( ui->VipEdit->text() );
    } catch ( std::exception &e ) {
        //! @todo add dialog with error
    }

    return QString::fromStdString( peer.serialize() );
}

PeerEditDialog::~PeerEditDialog()
{
    delete ui;
}

void PeerEditDialog::setData( const QString &name, const QString &vip, const QString &ip )
{
    ui->nameEdit->setText( name );
    ui->ipEdit->setText( ip );
    ui->VipEdit->setText( vip );
    ui->inviteEdit->setText( createInvitation() );
}

void PeerEditDialog::on_QrScanButton_clicked()
{
    //! @todo implement
}

void PeerEditDialog::on_QrZoomButton_clicked()
{
    //! @todo implement

}

void PeerEditDialog::on_VipEdit_editingFinished()
{
    //! @todo implement


    ui->inviteEdit->setText( createInvitation() );
}

QString PeerEditDialog::createInvitation()
{
    QString ip = ui->ipEdit->text();
    QString vip = ui->VipEdit->text();

    if( ip.size() > 0 ) {
        if( QHostAddress( ip ).protocol() != QAbstractSocket::IPv4Protocol ) {
            qDebug()<<"ip is improper";
            ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( false );
            return QString();
        }
    } else {
        ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( false );
        return QString ();
    }

    if( vip>0 ) {
        if( QHostAddress( vip ).protocol() != QAbstractSocket::IPv6Protocol ) {
            qDebug()<<"vip is improper";
            ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( false );
            return QString();
        }
    } else {
        ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( false );
        return QString();
    }

    ui->buttonBox->button( ui->buttonBox->Ok )->setEnabled( true );

    QString invitation = vip+"@"+ip;
    return invitation;
}

void PeerEditDialog::on_ipEdit_editingFinished()
{
    //! @todo add check data

    ui->inviteEdit->setText( createInvitation() );
}

void PeerEditDialog::on_buttonBox_accepted()
{
    emit editPeer( getPeerString() );
}


QString PeerEditDialog::getName()
{
    return ui->nameEdit->text();
}
QString PeerEditDialog::getIp()
{
    return ui->ipEdit->text();
}

QString PeerEditDialog::getVip()
{
    return ui->VipEdit->text();
}

QString PeerEditDialog::getInvitation()
{
    return createInvitation();
}
