#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);



	QTranslator qtTranslator;
	  qtTranslator.load("qt_" + QLocale::system().name(),
			  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	  a.installTranslator(&qtTranslator);

	  QTranslator myappTranslator;
	  myappTranslator.load("transProbe_pl.qm");		//file with translation - nesseesry loadning
	  a.installTranslator(&myappTranslator);


	MainWindow w;
	w.show();

	return a.exec();
}
