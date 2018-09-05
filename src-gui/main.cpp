#include "mainwindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <sodium.h>
#include <tuntaprunner.h>

int main( int argc, char *argv[] ) {
    try {
        if ( sodium_init() == -1 ) {
            qDebug() << "sodium init error";
            return 1;
        }


        QCoreApplication::setOrganizationName( "Yedino" );
        QCoreApplication::setOrganizationDomain( "https://github.com/yedino/galaxy42" );
        QCoreApplication::setApplicationName( "galaxy-gui" );

        QApplication a( argc, argv );
        QApplication::setWindowIcon(QIcon(":/main-icon"));

        MainWindow w;
        w.show();

        return a.exec();
    } catch ( const std::exception &ex ) {
        qDebug() << ex.what();
        return 1;
    } catch ( ... ) {
        qDebug() << "Unknown exception";
        return 2;
    }
}
