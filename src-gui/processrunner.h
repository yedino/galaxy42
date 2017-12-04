#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QObject>

class processRunner : public QObject
{
    Q_OBJECT
public:
    explicit processRunner(QObject *parent = nullptr);

    void initSettings();
    void loadSettings();

    bool startTunserver();

signals:

public slots:


};

#endif // PROCESSRUNNER_H
