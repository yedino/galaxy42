#ifndef ADDRESSBOOKFORM_H
#define ADDRESSBOOKFORM_H

#include <QWidget>
#include "addressmodel.h"

namespace Ui {
class addressBookForm;
}

class addressBookForm : public QWidget
{
    Q_OBJECT
    addressModel m_model;
public:
    explicit addressBookForm(QWidget *parent = 0);
    ~addressBookForm();

    void loadPeersFromFiles(const QString &file_name);
    void savePeersToFile(const QString &file_name);

private:
    Ui::addressBookForm *ui;
};

#endif // ADDRESSBOOKFORM_H
