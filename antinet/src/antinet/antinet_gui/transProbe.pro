#-------------------------------------------------
#
# Project created by QtCreator 2016-04-07T10:32:28
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = transProbe
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp \
    mydelegate.cpp \
    datamodel.cpp

HEADERS  += mainwindow.h \
    dialog.h \
    mydelegate.h \
    datamodel.h

FORMS    += mainwindow.ui \
    dialog.ui

TRANSLATIONS += transProbe_pl.ts

RESOURCES += \
    res.qrc
