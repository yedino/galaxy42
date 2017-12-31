#include "addpeerdialog.h"
#include "ui_addpeerdialog.h"

AddPeerDialog::AddPeerDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::AddPeerDialog )
{
    ui->setupUi( this );
}

AddPeerDialog::~AddPeerDialog()
{
    delete ui;
}

void AddPeerDialog::on_buttonBox_accepted()
{

}
