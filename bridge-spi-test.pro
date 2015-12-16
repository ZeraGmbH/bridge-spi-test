#-------------------------------------------------
#
# Project created by QtCreator 2015-12-07T16:35:26
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = bridge-spi-test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    czerabridgespi.cpp \
    cspidevice.cpp

HEADERS += \
    czerabridgespi.h \
    qspidevice.h \
    qspidevice_p.h

target.path = /usr/bin
INSTALLS += target
