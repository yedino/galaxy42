#include <QMessageBox>
#include <QErrorMessage>

#include "statusform.h"
#include "ui_statusform.h"


StatusForm::StatusForm( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::StatusForm )
{
    ui->setupUi( this );

    connect( &m_timer,SIGNAL( timeout() ),this,SLOT( onTickTimmer() ) );
    connect( &m_reconnectTimmer,SIGNAL( timeout() ),this,SLOT( onReConnectTimmer() ) );
    m_is_working = false;
}

bool StatusForm::isWorking()
{
    return m_is_working;
}

StatusForm::~StatusForm()
{
    delete ui;
}

void StatusForm::onTickTimmer()
{
    if( m_executor ) {
        // m_executor->ping();
    } else {
        qDebug()<<"no executor in statusForm";
    }
}

void StatusForm::onReConnectTimmer()
{
    ui->statusInycator->setChecked( false );
    ui->statusInycator->setText( tr( "connecting..." ) );
    ui->statusInycator->setStatusTip( tr( "connecting..." ) );
}

void StatusForm::onReConnect()
{
//		rozpocznij polaczenie - jesli nie rozpocznij dzialanie reconect timer
}


void StatusForm::onConnectionSuccess()
{
    // ustaw stan na wlaczony
    ui->statusInycator->setChecked( true );
    ui->statusInycator->setStatusTip( tr( "connected - no rpc server dectected" ) );
    ui->statusInycator->setText( tr( "connected - no node" ) );

    if( m_is_working ) {
        m_is_working = false;
        emit netConnect( false );
    }
}

void StatusForm::onLostConnection()
{

    QErrorMessage msg ;
    msg.showMessage( QString( tr( "connection lost" ) ) );
    ui->statusInycator->setStatusTip( tr( "unconnected" ) );
    ui->statusInycator->setChecked( false );
    ui->statusInycator->setText( tr( "unconnected" ) );

    if( m_is_working ) {
        m_is_working = false;
        emit netConnect( false );
    }
}

void StatusForm::onErrorOccured( QString err )
{
    QErrorMessage msg;
    msg.showMessage( QString( err ) );
    msg.exec();

}

void StatusForm::onGetSessionId()
{
    ui->statusInycator->setChecked( true );
    ui->statusInycator->setStatusTip( tr( "got rpc id" ) );
    ui->statusInycator->setText( tr( "got rpc id" ) );
    m_is_working = true;
    emit netConnect( m_is_working );
}

void StatusForm::on_pushButton_clicked()
{
    if( ! m_executor ) {
        onErrorOccured( tr( "can't create connection - internal error" ) );
        return;
    }

    m_executor->resetConnection();
    m_is_working = false;
    emit netConnect( m_is_working );
}
