#ifndef QRDIALOG_H
#define QRDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
namespace Ui {
class QrDialog;
}


/**
 * @brief The QrDialog class klasa reprezentujaca dialog qr kodu .
 */
class QrDialog : public QDialog
{
    Q_OBJECT
    QGraphicsScene m_scene;		//! scena na ktorej malowany jest obrazek
    QPixmap m_code;				//! obrazek z qr codem

public:
    explicit QrDialog(QPixmap &qr_code,QWidget *parent = 0);	//! konstruktor przyjmujacy i kopiujacy qr kod
    ~QrDialog();

private:
    Ui::QrDialog *ui;
};

#endif // QRDIALOG_H
