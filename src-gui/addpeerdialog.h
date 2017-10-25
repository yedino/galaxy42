#ifndef ADDPEERDIALOG_H
#define ADDPEERDIALOG_H

#include <QDialog>

namespace Ui {
class AddPeerDialog;
}

class AddPeerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPeerDialog(QWidget *parent = 0);
    ~AddPeerDialog();

private slots:
    void on_buttonBox_accepted();


private:
    Ui::AddPeerDialog *ui;
};

#endif // ADDPEERDIALOG_H
