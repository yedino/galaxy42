TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += --std=c++11

SOURCES += main.cpp \
    dhtnode.cpp \
    dht_world.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    dhtnode.h \
    dht_world.h

