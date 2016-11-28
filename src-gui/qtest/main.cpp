#include <QtTest>
#include "t_dataeater.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

	Test_dataeater t_dataeater;
    
    // multiple test suites can be ran like this
    //return QTest::qExec(&testSuite1, argc, argv) |
	//        QTest::qExec(&testSuite2, argc, argv);
	return QTest::qExec(&t_dataeater, argc, argv);
}
