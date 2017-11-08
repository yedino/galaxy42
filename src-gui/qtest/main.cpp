#include <QtTest/QTest>
#include "t_dataeater.hpp"
#include "test_meshpeer.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    Test_dataeater t_dataeater;
    Test_Meshpeer t_meshpeer;
    
    // multiple test suites can be ran like this
    //return QTest::qExec(&testSuite1, argc, argv) |
    //        QTest::qExec(&testSuite2, argc, argv);
    QTest::qExec(&t_meshpeer,argc,argv);
    return QTest::qExec(&t_dataeater, argc, argv);
}
