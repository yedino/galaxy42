#ifndef GET_HOST_INFO_HPP
#define GET_HOST_INFO_HPP

#include <QDialog>

namespace Ui {
class Dialog;
}

class hostDialog : public QDialog {
	Q_OBJECT

public:
	explicit hostDialog(QWidget *parent = 0);
	~hostDialog();

	QString getHost();
	uint16_t getPort();
signals:
	void host_info(QString, uint16_t);

private slots:
	void on_buttonBox_accepted();

private:
	Ui::Dialog *ui;
};

#endif // GET_HOST_INFO_HPP
