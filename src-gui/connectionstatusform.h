#ifndef CONNECTIONSTATUSFORM_H
#define CONNECTIONSTATUSFORM_H

#include <QWidget>

namespace Ui {
class ConnectionStatusForm;
}

class ConnectionStatusForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionStatusForm(QWidget *parent = 0);
    ~ConnectionStatusForm();

private:
    Ui::ConnectionStatusForm *ui;
};

#endif // CONNECTIONSTATUSFORM_H
