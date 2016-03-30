#ifndef XBEEGUI_H
#define XBEEGUI_H

#include <QMainWindow>
#include <QTimer>
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
    int testInt;
    KBridgeDll myDmxBridge;
    //QextSerialPort *port;
    QSerialPort *port;
    QMap<QString,kDevice*> kDevices;
    QList <QString> knownList;
    Flashbuffer* myflash;
    Flashbuffer* remoteflash;
    void SendFlashProgramPacket();
    enum eeFlashProgrammingStates{
        eFPS_None=0,
        eFPS_LogInNecessary,
        eFPS_LoggingOf,
        eFPS_FlashReset,
        eFPS_Flash,
        eFPS_FlashComplete,
        eFPS_FlashCRC_request,
        eFPS_FlashCRC_compare,
        eFPS_FinalCRC
    };
    struct mykboot_t{
        qint32       progress;
        int          state;
        uint16_t     kframe;
        uint16_t     xbeeaddr;
        int          retries;
        qint32       receivedFlashCRC;
        QByteArray   lastSentPackage;
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
    void loadFlashHex(QString);
    void DiplayDisplayValues();
    void UpdateVersionList();
    bool bLogToFile;
    QFile* logfile;
private:
    eeMainstate pMainstate;
    void PacketForLogfile(QByteArray packet);
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
    void easySetRGB_ALL(uint8_t r,uint8_t g,uint8_t b,uint8_t l,uint8_t strobe);
    void on_pbDmxW_clicked();
    void on_pbDmxR_clicked();
    void on_pbDmxG_clicked();
    void on_pbDmxB_clicked();
    void on_pbDmxX_clicked();
    void on_pbBootLoadFlashDevelop_clicked();
    void on_lineKSEND_returnPressed();

    void on_chkLogToFile_stateChanged(int arg1);

    void on_pbDmxW10_clicked();

private:
    Ui::XBeeGuiClass ui;
};

#endif // XBEEGUI_H
