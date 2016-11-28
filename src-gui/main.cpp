#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	std::shared_ptr<MainWindow> w = MainWindow::create_shared_ptr();
	w->show();

	return a.exec();
}
