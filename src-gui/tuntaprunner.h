#ifndef TUNTAPRUNNER_H
#define TUNTAPRUNNER_H

#include <QObject>
#include <QProcess>

class TunTapRunner : public QObject
{
    Q_OBJECT

    void runTunsever(QString tunserver_path);
    void stopTunserver();

public:
    explicit TunTapRunner(QObject *parent,const QString& tunserver_path,const QString& script_path);

    void restartTunserver(QString tunserver_path ="");

signals:

    void errorNoty(QString e);

public slots:

    void onStateChanged(QProcess::ProcessState newState);
    void onError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onStart();
    void onReadyReadStandardError();
    void onReadyReadStandardOutput();

private:
    QProcess m_tuntap_process;
};

#endif // TUNTAPRUNNER_H
