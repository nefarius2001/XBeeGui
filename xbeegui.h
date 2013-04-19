#ifndef XBEEGUI_H
#define XBEEGUI_H

#include <QMainWindow>
#include <QTimer>
#include <QLibrary>
#include "ui_xbeegui.h"
#include "CComPort.h"
#include "CXBee.h"
#include "kBridgeDll.h"
//#include "qextserialport.h"
#include <QSettings>

class XBeeGui : public QMainWindow
{
    Q_OBJECT

public:
    QSettings settings;
    XBeeGui(QWidget *parent = 0);
    ~XBeeGui();
    void RefreshPorts();
    CXBee cPort;
    QTimer t1;
    QTimer t2;
    QLibrary * libKDmxBridge;
    DmxGetOutPointer_t DmxGetOutPointer;
    BYTE *pDmxData;
    //QextSerialPort *port;
    QtSerialPort *port;
private slots:
    void handleT1();
    void handleT2();
    void handlePbRefreshPorts();
    void handleLstPortsDblClick(const QModelIndex & item);
    void handleLstPortsClick(const QModelIndex & item);
    void handlePbTest();
    void handlePbTestRead();
    void handlePbTestWrite();
    void handleXBeeRx(XBeeApiMsg_t msg);
private:
    Ui::XBeeGuiClass ui;
};

#endif // XBEEGUI_H
