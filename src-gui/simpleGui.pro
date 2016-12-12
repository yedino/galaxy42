#-------------------------------------------------
#
# Project created by QtCreator 2016-10-14T10:46:39
#
#-------------------------------------------------

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4.8): QT += widgets

QMAKE_CC = gcc
QMAKE_CXX = g++
CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14

TARGET = simpleGui
TEMPLATE = app

LIBS += -lboost_system


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
    tunserver_process.cpp

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
    ../depends/json/src/json.hpp

FORMS += \
    mainwindow.ui \
    addressdialog.ui \
    debugdialog.ui \
    get_host_info.ui


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
        qtest/t_dataeater.hpp

    SOURCES += \
        qtest/main.cpp \

} else {
    message(Normal build)
}
