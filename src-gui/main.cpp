#include "mainwindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <sodium.h>

int main( int argc, char *argv[] ) {
    if ( sodium_init() == -1 ) {
        qDebug() << "sodium init error";
        return 1;
    }

    QCoreApplication::setOrganizationName( "Tigusoft" );
    QCoreApplication::setOrganizationDomain( "https://github.com/yedino/galaxy42" );
    QCoreApplication::setApplicationName( "galaxy-gui" );

    QApplication a( argc, argv );
//	std::shared_ptr<MainWindow> w = MainWindow::create_shared_ptr();
//    MainWindow *w = new MainWindow(&a);
//    w->show();
    MainWindow w;
    w.show();

    return a.exec();
}
