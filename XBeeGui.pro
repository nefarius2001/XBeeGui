
CONFIG += serialport
greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

#include(../../QtAddOns/qextserialport/src/qextserialport.pri)
include(../QtAddOns/combox/combox.pri)

TARGET = XBeeGui
TEMPLATE = app

QT += core  gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#include(../QtAddOns/qextserialport/src/qextserialport.pri)
#include(../QtAddOns/combox/combox.pri)
#include(../QtAddOns/qcustomplot/src/qcustomplot.pri)

HEADERS += \
    xbeegui.h \
    kDmxBridge.h \
    kBridgeDll.h \
    kdevice.h \
    flashbuffer.h
SOURCES += \
    main.cpp \
    xbeegui.cpp \
    kdevice.cpp \
    flashbuffer.cpp
FORMS += xbeegui.ui
#RESOURCES +=
