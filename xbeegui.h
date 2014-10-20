#ifndef XBEEGUI_H
#define XBEEGUI_H

#include <QMainWindow>
#include <QTimer>
#include <QLibrary>
#include "ui_xbeegui.h"
//#include "CComPort.h"
#include "CXBee.h"
#include "combox.h"
#include "kBridgeDll.h"
//#include "qextserialport.h"
#include <QSettings>
#include <QtSerialPort>
#include <QMap>
#include "kdevice.h"
#include "flashbuffer.h"

class XBeeGui : public QMainWindow
{
    Q_OBJECT

public:
    QSettings settings;
    XBeeGui(QWidget *parent = 0);
    ~XBeeGui();
    CXBee myXbee;
    QTimer t1;
    QTimer t2;
    QTimer t3;
    QLibrary * libKDmxBridge;
    DmxGetOutPointer_t DmxGetOutPointer;
    BYTE *pDmxData;
    //QextSerialPort *port;
    QSerialPort *port;
    QMap<QString,kDevice*> kDevices;
    Flashbuffer* myflash;
    void SendFlashProgramPacket();
    enum eeFlashProgrammingStates{
        eFPS_None=0,
        eFPS_LogInNecessary,
        eFPS_LoggingOf,
        eFPS_FlashReset,
        eFPS_Flash,
        eFPS_FlashComplete,
        eFPS_FlashCRC
    };
    struct mykboot_t{
        qint32       progress;
        char         state;
        uint16_t     kframe;
        uint16_t     xbeeaddr;
        int          retries;
        qint32       receivedFlashCRC;
    }mykboot;
    quint16 GetSelectedXBeeAddr_uint16();
    QString GetSelectedXBeeAddr_string();
    void SetSelectedXBee(QString sKeyAddr);
    enum eeMainstate{
        eMsNormal,
        eMsBootloader,
        eMsAutoFlash
    };
    void ChangeMainstate(eeMainstate newState);
    eeMainstate GetMainState(void);
    void loadFlashHex();
    void DiplayDisplayValues();
private:
    eeMainstate pMainstate;
private slots:
    void handleT1();
    void handleT2();
    void handleT3();
    void handlePbTestRead();
    void handlePbTestWrite();
    void handleXBeeRx(QByteArray);
    void handleXbeeTx(QByteArray);
    void handleComOpened();
    void on_pbKpngBroadcast_clicked();
    void on_pbKpngKnown_clicked();
    void on_displayValues_clicked(const QModelIndex &index);
    void on_listDevices_itemSelectionChanged();
    void on_pbKBoot_clicked();
    void on_pbKbootLogout_clicked();
    void on_pbBootTest_clicked();
    void on_pbTest_clicked();

    void on_pbBootChipReset_clicked();

    void on_pbBootLoadFlash_clicked();

    void on_pbBootFlashProgram_clicked();

    void on_pbBootRequestCrc_clicked();

    void on_chkAutoFirmware_stateChanged(int newstate);

private:
    Ui::XBeeGuiClass ui;
};

#endif // XBEEGUI_H
