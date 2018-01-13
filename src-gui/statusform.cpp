#include <QMessageBox>
#include <QErrorMessage>

#include "statusform.h"
#include "ui_statusform.h"


StatusForm::StatusForm( QWidget *parent ) :
    QWidget( parent ),
    ui( new Ui::StatusForm )
{
    ui->setupUi( this );
    connect( &m_timer,SIGNAL( timeout() ),this,SLOT( onTickTimmer() ) );				// no check ping timmer yet
    connect( &m_reconnectTimer,SIGNAL( timeout() ),this,SLOT( onReConnectTimmer() ) );	//no reconnect timer yet
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
//    ui->statusIndicator->setChecked( false );
    ui->statusIndicator->setText( tr( "connecting..." ) );
    ui->statusIndicator->setStatusTip( tr( "connecting..." ) );

}

void StatusForm::onReConnect()
{
//		rozpocznij polaczenie - jesli nie rozpocznij dzialanie reconect timer
}


void StatusForm::onConnectionSuccess()
{
    // ustaw stan na wlaczony
//    ui->statusIndicator->setChecked( true );
    ui->statusIndicator->setStatusTip( tr( "connected - no rpc server detecected" ) );
    ui->statusIndicator->setText( tr( "connected - no node" ) );

    if( m_is_working ) {
        m_is_working = false;
//        emit netConnect( false );
    }
}

void StatusForm::onLostConnection()
{

    QErrorMessage msg ;
    msg.showMessage( QString( tr( "connection lost" ) ) );
    ui->statusIndicator->setStatusTip( tr( "disconnected" ) );
//    ui->statusIndicator->setChecked( false );
    ui->statusIndicator->setText( tr( "disconnected" ) );

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
//    ui->statusIndicator->setChecked( true );
    ui->statusIndicator->setStatusTip( tr( "got rpc id" ) );
    ui->statusIndicator->setText( tr( "RPC connected" ) );
    m_is_working = true;
    emit netConnect( m_is_working );
}

void StatusForm::setDebugInfo(const QString& str)
{
    ui->statusIndicator->setText(str);
}

void StatusForm::on_pushButton_clicked()
{
    if( ! m_executor ) {
        onErrorOccured( tr( "can't create connection - internal error" ) );
        return;
    }

    m_executor->resetConnection();
    m_is_working = false;
//    emit netConnect( m_is_working );
}


void StatusForm::setStatus( QString status)
{
    ui->statusIndicator->setText(status);
}

