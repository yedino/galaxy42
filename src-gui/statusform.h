#ifndef STATUSFORM_H
#define STATUSFORM_H

#include <QWidget>
#include <QTimer>
#include <commandexecutor.hpp>

namespace Ui {
class StatusForm;
}

class StatusForm : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusForm(QWidget *parent = 0);
    ~StatusForm();
    inline void setExecutor(commandExecutor *executor){m_executor = executor ;}

public slots:
   void onTickTimmer();
    void onReConnectTimmer();
    void onReConnect();
    void onErrorOccured(QString err);
    void onConnectionSuccess();
    void onLostConnection();
    void onGetSessionId();

private:

    commandExecutor *m_executor = nullptr;
    QTimer m_timer;
    QTimer m_reconnectTimmer;

    Ui::StatusForm *ui;
};

#endif // STATUSFORM_H
