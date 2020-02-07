#-------------------------------------------------
#
# Project created by QtCreator 2015-12-07T16:35:26
#
#-------------------------------------------------

QT       += core
QT       += spidevice bridgefmtspihelper

QT       -= gui

TARGET = bridge-spi-test
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app


SOURCES += main.cpp

HEADERS +=

target.path = /usr/bin
INSTALLS += target

exists(localpaths.user.pri) {
    include(localpaths.user.pri)
}
