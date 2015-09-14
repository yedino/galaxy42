#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T14:00:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = antinet_qt
TEMPLATE = app

release: DESTDIR = build/release
debug:   DESTDIR = build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

CONFIG += c++11

SOURCES += main.cpp\
        c_antinet_agent_api.cpp \
        c_antinet_agent_api_msg.cpp \
        c_antinet_agent_api_msg_success.cpp \
        c_antinet_agent_api_msg_failure.cpp \
        c_antinet_agent_api_msg_info.cpp \
        c_antinet_agent_bank.cpp

HEADERS += c_antinet_agent_api.hpp \
    c_antinet_agent_api_msg.hpp \
    c_antinet_agent_api_msg_failure.hpp \
    c_antinet_agent_api_msg_info.hpp \
    c_antinet_agent_api_msg_success.hpp \
    c_antinet_agent_bank.hpp

FORMS    += c_antinet_agent_api.ui
