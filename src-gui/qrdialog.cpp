#include "qrdialog.h"
#include "ui_qrdialog.h"

QrDialog::QrDialog( QPixmap &qr_code,QWidget *parent ) :
    QDialog( parent ),m_code( qr_code ),ui( new Ui::QrDialog )
{
    ui->setupUi( this );

    m_scene.addPixmap( m_code );
    ui->QrView->setScene( &m_scene );
}

QrDialog::~QrDialog()
{
    delete ui;
}
