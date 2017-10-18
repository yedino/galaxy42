#include <QSettings>

#include "get_host_info.hpp"
#include "ui_get_host_info.h"

hostDialog::hostDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::Dialog )
{
    ui->setupUi( this );

    ui->host_lineEdit->setText( QSettings().value( "rpcConnection/Ip" ).toString() );
    ui->port_lineEdit->setText( QSettings().value( "rpcConnection/port" ).toString() );
}

hostDialog::~hostDialog()
{
    delete ui;
}

QString hostDialog::getHost() {
    return ui->host_lineEdit->text();
}

uint16_t hostDialog::getPort() {
    return ui->port_lineEdit->text().toUInt();
}

void hostDialog::on_buttonBox_accepted() {
    emit host_info( ui->host_lineEdit->text(),
                    ui->port_lineEdit->text().toUInt() );
}
