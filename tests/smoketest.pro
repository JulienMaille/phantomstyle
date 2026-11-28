QT += core gui widgets

TARGET = smoketest
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 console

include(../src/phantom/phantom.pri)

SOURCES += smoketest.cpp
