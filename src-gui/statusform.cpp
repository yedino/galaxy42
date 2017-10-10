#include <QMessageBox>
#include <QErrorMessage>

#include "statusform.h"
#include "ui_statusform.h"


StatusForm::StatusForm(QWidget *parent) :
    QStatusBar(parent),
    ui(new Ui::StatusForm)
{
    ui->setupUi(this);

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(onTickTimmer()));
    connect(&m_reconnectTimmer,SIGNAL(timeout()),this,SLOT(onReConnectTimmer()));
}

StatusForm::~StatusForm()
{
    delete ui;
}

void StatusForm::onTickTimmer()
{
    if( m_executor ){
        // m_executor->ping();
    }else{
        qDebug()<<"no executor in statusForm";
    }
}

void StatusForm::onReConnectTimmer()
{
    ui->statusInycator->setChecked(false);
    ui->statusInycator->setText("connecting...");
    ui->statusInycator->setStatusTip("connecting...");
//	m_executor->	zresetuj polaczenie
}

void StatusForm::onReConnect()
{
//		rozpocznij polaczenie - jesli nie rozpocznij dzialanie reconect timer
}


void StatusForm::onConnectionSuccess()
{
    // ustaw stan na wlaczony
    ui->statusInycator->setChecked(true);
    ui->statusInycator->setStatusTip("connected");
    ui->statusInycator->setText("connected");
}

void StatusForm::onLostConnection()
{
    QErrorMessage msg ;
    msg.showMessage(QString("connection lost"));
    ui->statusInycator->setStatusTip("unconnected");
    ui->statusInycator->setChecked(false);
    ui->statusInycator->setText("unconnected");
}

void StatusForm::onErrorOccured(QString err)
{
    QErrorMessage msg;
    msg.showMessage(QString(err));
}

void StatusForm::onGetSessionId()
{
    ui->statusInycator->setChecked(true);
    ui->statusInycator->setStatusTip("got rpc id");
    ui->statusInycator->setText("got rpc id");
}
