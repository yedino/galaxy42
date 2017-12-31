#include <QSettings>
#include <QMessageBox>


#include "get_host_info.hpp"
#include "ui_get_host_info.h"

hostDialog::hostDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::Dialog )
{
    ui->setupUi( this );

    ui->host_lineEdit->setText( QSettings().value( "rpcConnection/Ip" ).toString() );
    ui->port_lineEdit->setText( QSettings().value( "rpcConnection/port" ).toString() );



    QSettings settings;

    ui->tuntapPathLine->setText( settings.value("tuntap_path").toString());
    ui->tuntapLine->setText( settings.value("tuntap_name").toString());
    ui->sudoScriptLine->setText(settings.value("sudo_script").toString());
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

void hostDialog::on_pushButton_clicked()
{
    QString script_path = ui->sudoScriptLine->text();
    QString tuntat_path = ui->tuntapPathLine->text();
    QString tuntap_program = ui->tuntapLine->text();

    QSettings sett;

    sett.setValue("tuntap_path",script_path);
    sett.setValue("tuntap_name",tuntat_path);
    sett.setValue("sudo_script",tuntap_program);

    QMessageBox endBox(QMessageBox::Information,tr("reset program"),tr("You need restart program to feel change"));
    endBox.exec();

}

