TEMPLATE = app
TARGET = XBeeGui
QT += core  gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QT += serialport
#CONFIG += console
CONFIG += serialport
greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}
#include(../QtAddOns/qextserialport/src/qextserialport.pri)
include(../QtAddOns/combox/combox.pri)
#include(../QtAddOns/qcustomplot/src/qcustomplot.pri)

HEADERS += CXBee.h \
    CComPort.h \
    xbeegui.h \
    kDmxBridge.h \
    kBridgeDll.h
SOURCES += CXBee.cpp \
    CComPort.cpp \
    main.cpp \
    xbeegui.cpp
FORMS += xbeegui.ui
#RESOURCES +=
#CONFIG += console
