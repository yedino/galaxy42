#include "mainwindow.hpp"
#include <QApplication>
#include <sodium.h>

int main(int argc, char *argv[]) {
	if (sodium_init() == -1) {
		return 1;
	}
	QApplication a(argc, argv);
	std::shared_ptr<MainWindow> w = MainWindow::create_shared_ptr();
	w->show();

	return a.exec();
}
