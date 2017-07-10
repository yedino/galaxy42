#include "mainwindow.hpp"
#include <QApplication>
#include <sodium.h>

int main(int argc, char *argv[]) {
	if (sodium_init() == -1) {
		qDebug() << "sodium init error";
		return 1;
	}
	QApplication a(argc, argv);
	std::shared_ptr<MainWindow> w = MainWindow::create_shared_ptr();
	w->show();
//	MainWindow w;
//	w.show();

	return a.exec();
}
