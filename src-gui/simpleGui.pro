#-------------------------------------------------
#
# Project created by QtCreator 2016-10-14T10:46:39
#
#-------------------------------------------------

QT       += core gui network multimedia
MOBILITY += multimedia
greaterThan(QT_MAJOR_VERSION, 4.8): QT += widgets

QMAKE_CC = gcc
QMAKE_CXX = g++
CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic

TARGET = simpleGui
TEMPLATE = app

INCLUDEPATH = ../depends/json/src
#LIBS += -lboost_system -lsodiu

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    addressdialog.cpp \
    paramscontainer.cpp \
    commandparser.cpp \
    dataeater.cpp \
    debugdialog.cpp \
    netclient.cpp \
    commandexecutor.cpp \
    get_host_info.cpp \
    tunserver_process.cpp \
    quickstartdialog.cpp \
    connectionstatusform.cpp \
    qrdialog.cpp \
    meshpeer.cpp \
    peersmodel.cpp \
    order.cpp \
        peerlistform.cpp \
    addpeerdialog.cpp\
        peereditdialog.cpp \
    rpccounter.cpp \
    sendedcommand.cpp \
    commandsender.cpp \
    nodecontrolerdialog.cpp \
    statusform.cpp \
    processrunner.cpp \
    addressmodel.cpp \
    addressbookform.cpp \
    tuntaprunner.cpp


HEADERS += \
    mainwindow.hpp \
    addressdialog.hpp \
    paramscontainer.hpp \
    commandparser.hpp \
    dataeater.hpp \
    commandexecutor.hpp \
    debugdialog.hpp \
    netclient.hpp \
    get_host_info.hpp \
    tunserver_process.hpp \
    quickstartdialog.h \
    peerlistform.h \
    connectionstatusform.h \
    qrdialog.h \
    meshpeer.h \
    order.hpp\
        peersmodel.h \
    addpeerdialog.h\
        peereditdialog.h \
    rpccounter.h \
    sendedcommand.h \
    commandsender.h \
    nodecontrolerdialog.h \
    statusform.h \
    processrunner.h \
    addressmodel.h \
    addressbookform.h \
    tuntaprunner.h

FORMS += \
    mainwindow.ui \
    addressdialog.ui \
    debugdialog.ui \
    get_host_info.ui \
    quickstartdialog.ui \
    connectionstatusform.ui \
    qrdialog.ui \
    quickstartform.ui \
        peerlistform.ui \
    peereditdialog.ui\
        addpeerdialog.ui \
    nodecontrolerdialog.ui \
    statusform.ui \
    addressbookform.ui \
    addressbookform.ui


debug {
	DESTDIR = build/debug
}

release {
	DESTDIR = build/release
}


test {
    message(Test build)

    QT += testlib
    TARGET = test_simpleGui
    SOURCES -= main.cpp

    HEADERS += \
        qtest/t_dataeater.hpp\
        qtest/test_meshpeer.h

    SOURCES += \
        qtest/main.cpp \

} else {
    message(Normal build)
}


macos {
        LIBS += -L /usr/local/lib -lsodium
        INCLUDEPATH += /usr/local/include/
        INCLUDEPATH += $$PWD/../depends/json/src/

}

win32: LIBS += -L$$PWD/../../repo/libsodium-win32/lib/ -lsodium
linux: LIBS += -lsodium

win32:{
        INCLUDEPATH += $$PWD/../../repo/libsodium-win32/include # path to sodium manual set
        DEPENDPATH += $$PWD/../../repo/libsodium-win32/include	#path to sodium manual path
}

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../repo/libsodium-win32/lib/sodium.lib # path to sodium manual se
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../repo/libsodium-win32/lib/libsodium.a  #path to sodium manual path

TRANSLATIONS = hellotr_pl.ts


RESOURCES += \
    icons.qrc \
    sounds.qrc

