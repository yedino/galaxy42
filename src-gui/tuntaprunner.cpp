#include "tuntaprunner.h"
#include <QProcess>
#include <QDebug>


TunTapRunner::TunTapRunner(QObject *parent,const QString& tunserver_path,const QString& script_path) : QObject(parent)
{
    connect(&m_tuntap_process,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(onError(QProcess::ProcessError)));
    connect(&m_tuntap_process,SIGNAL(finished(int , QProcess::ExitStatus )),this,SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(&m_tuntap_process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStandardError()));
    connect(&m_tuntap_process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStandardOutput()));
    connect(&m_tuntap_process,SIGNAL(started()),this,SLOT(onReadyReadStandardOutput()));
    connect(&m_tuntap_process,SIGNAL(stateChanged(QProcess::ProcessState )),this,SLOT(onStateChanged(QProcess::ProcessState)));

    m_tuntap_process.execute(script_path,QStringList{tunserver_path});
}


void TunTapRunner::onStateChanged(QProcess::ProcessState newState)
{
    qDebug()<<"tuntap new state:"<<newState;
}

void TunTapRunner::onError(QProcess::ProcessError error)
{
    qDebug()<<"tuntap error:"<<error;
}

void TunTapRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug()<<"tuntap stoped with code"<< exitCode<<" with status: "<<exitStatus;
}

void TunTapRunner::onStart()
{
    qDebug()<<"tuntap started";
}

void TunTapRunner::onReadyReadStandardError()
{
    qDebug()<<"tuntap error: " <<m_tuntap_process.readAllStandardError();
}

void TunTapRunner::onReadyReadStandardOutput()
{
    qDebug()<<"tuntap said: "<<m_tuntap_process.readAllStandardOutput();
}

void TunTapRunner::runTunsever(QString tunserver_path)
{
//    m_tuntap_process.execute(tunserver_path);
}

void TunTapRunner::stopTunserver()
{
//    m_tuntap_process.kill();
}
