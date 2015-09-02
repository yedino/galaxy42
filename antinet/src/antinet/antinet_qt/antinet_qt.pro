#-------------------------------------------------
#
# Project created by QtCreator 2015-08-31T14:00:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = antinet_qt
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        c_antinet_agent_api.cpp \
    c_antinet_agent_api_msg.cpp \
    c_antinet_agent_api_msg_success.cpp \
    c_antinet_agent_api_msg_failure.cpp \
    c_antinet_agent_api_msg_info.cpp

HEADERS  += c_antinet_agent_api.h \
    c_antinet_agent_api_msg.h \
    c_antinet_agent_api_msg_success.h \
    c_antinet_agent_api_msg_failure.h \
    c_antinet_agent_api_msg_info.h

FORMS    += c_antinet_agent_api.ui
