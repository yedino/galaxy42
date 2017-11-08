#include "connectionstatusform.h"
#include "ui_connectionstatusform.h"

ConnectionStatusForm::ConnectionStatusForm( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::ConnectionStatusForm )
{
    ui->setupUi( this );
}

ConnectionStatusForm::~ConnectionStatusForm()
{
    delete ui;
}
