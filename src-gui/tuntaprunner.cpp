#include "tuntaprunner.h"
#include <QProcess>
#include <QDebug>


TunTapRunner::TunTapRunner(QObject *parent) : QObject(parent)
{

//    QString script_path = "C:\\Users\\nautic\\galaxy42\\win_startscript.bat";
    QString script_path = "C:\\Users\\nautic\\galaxy42\\build-simpleGui-Desktop_Qt_5_9_1_MinGW_32bit-Debug\\build\\debug\\getadmin.vbs";
    QString tunserver_path = "C:\\Users\\nautic\\galaxy42\\tunserver.elf.exe";

    connect(&m_tuntap_process,SIGNAL(errorOccurred(QProcess::ProcessError error)),this,SLOT(onError(QProcess::ProcessError)));
    connect(&m_tuntap_process,SIGNAL(finished(int exitCode, QProcess::ExitStatus exitStatus)),this,SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(&m_tuntap_process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStandardError()));
    connect(&m_tuntap_process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStandardOutput()));
    connect(&m_tuntap_process,SIGNAL(started()),this,SLOT(onReadyReadStandardOutput()));
    connect(&m_tuntap_process,SIGNAL(stateChanged(QProcess::ProcessState newState)),this,SLOT(onStateChanged(QProcess::ProcessState newState)));

//ShellExecute(NULL, "open",tunserver_path.toStdString().c_str() , NULL, NULL, SW_SHOWNORMAL);
//    system(tunserver_path.toStdString().c_str());
    m_tuntap_process.execute(script_path,QStringList{tunserver_path});
//    m_tuntap_process.execute(tunserver_path);
}


void TunTapRunner::onStateChanged(QProcess::ProcessState newState)
{
    qDebug()<<"new state"<<newState;
}

void TunTapRunner::onError(QProcess::ProcessError error)
{
    qDebug()<<"error:"<<error;
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
    qDebug()<<m_tuntap_process.readAllStandardOutput();
}

void TunTapRunner::runTunsever(QString tunserver_path)
{
//    m_tuntap_process.execute(tunserver_path);
}

void TunTapRunner::stopTunserver()
{
//    m_tuntap_process.kill();
}
