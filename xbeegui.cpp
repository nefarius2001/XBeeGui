#include "xbeegui.h"
//#include <QtSerialPort>
//#include <QtAddOnSerialPort/serialport.h>
//#include <QtAddOnSerialPort/serialportinfo.h>
#include <QtCore>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QtDebug>
#include <QString>
#include <stdio.h>
//#include "serialport.h"
#include <combox.h>

#define SENDKDMX_NUMBEROFVALUES 40
#define Byte3(xxx) ((xxx >>(8*3)) & 0xFF)
#define Byte2(xxx) ((xxx >>(8*2)) & 0xFF)
#define Byte1(xxx) ((xxx >>(8*1)) & 0xFF)
#define Byte0(xxx) ((xxx >>(8*0)) & 0xFF)

#define TimestampFormat "hh:mm:ss.zzz"

#define BOOTLOADER_MAINAPP_SIZE 0x00018000

XBeeGui::~XBeeGui()
{
    delete remoteflash; remoteflash=0;
    qDebug("~XBeeGui");
    delete port;
    port=NULL;
}

XBeeGui::XBeeGui(QWidget *parent)
    : QMainWindow(parent)
{
    QLibrary l;
    QString sPort;
    qDebug("XBeeGui");
    int iFor;

        this->pMainstate=eMsNormal;
        this->myflash=0;
        //this->remoteflash=new Flashbuffer(0x08008000, (0x00040000-0x8000));
        this->remoteflash=new Flashbuffer(0x08008000, BOOTLOADER_MAINAPP_SIZE);
        qDebug()<< QString().sprintf("remoteflash empty CRC 0x%08X",remoteflash->GetCrc());
        mykboot.progress=0;
        mykboot.state=0;
        mykboot.kframe=0;
        mykboot.receivedFlashCRC=0;
        mykboot.retries=0;
        mykboot.xbeeaddr=0;

        ui.setupUi(this);
        port=new QSerialPort(this);
        ui.mycombox->setComPort(port);

        connect(ui.mycombox, SIGNAL(ComOpened()), this, SLOT(handleComOpened()));
        connect(ui.pbTest, SIGNAL(released()) , this, SLOT(handlePbTest()));
        connect(ui.pbTestRead, SIGNAL(released()) , this, SLOT(handlePbTestRead()));
        connect(ui.pbTestWrite, SIGNAL(released()) , this, SLOT(handlePbTestWrite()));
        connect(&this->myXbee,SIGNAL(XbeeMsgRx(QByteArray)),this,SLOT(handleXBeeRx(QByteArray)));
        connect(&this->myXbee,SIGNAL(XbeeTxUsart(QByteArray)),this,SLOT(handleXbeeTx(QByteArray)));
        connect(&this->t1, SIGNAL(timeout()), this, SLOT(handleT1()));
        connect(&this->t2, SIGNAL(timeout()), this, SLOT(handleT2()));
        connect(&this->t3, SIGNAL(timeout()), this, SLOT(handleT3()));
        this->t1.setSingleShot(false);
        this->t2.setSingleShot(false);
        this->t3.setSingleShot(false);
        this->t1.start(1);
        this->t2.start(1000);
        this->t3.start(100);
        qDebug("Load kBridgeDll");
        DmxGetOutPointer=0;
        pDmxData=0;
        QString sdll= QString("kBridgeDll");
        //QString sdll=QString ( getenv("WINDIR") ) + "\\system32\\kBridgeDll.dll";
        qDebug()<<sdll;
        libKDmxBridge=new QLibrary(sdll);

        if(libKDmxBridge->load()==false){
            qDebug("kBridgeDll NOT loaded");
        }else{
            qDebug("kBridgeDll loaded");
            DmxGetOutPointer = (DmxGetOutPointer_t) libKDmxBridge->resolve("DmxGetOutPointer");
            if (DmxGetOutPointer){
                qDebug("DmxGetOutPointer loaded");
                pDmxData = DmxGetOutPointer();
                QString s;
                s.sprintf("pDmxData loaded(&%li)",(long)pDmxData);
                qDebug() << s;
            }else{
                qWarning("DmxGetOutPointer not resolved");
            }
        }

        QStandardItemModel *model = new QStandardItemModel(4,2,this); //2 Rows and 3 Columns
        model->setHorizontalHeaderItem(0, new QStandardItem(QString("Property")));
        model->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));
        QStringList tmpslist;
        model->setItem(0,0,new QStandardItem("NAME")); tmpslist.append("Name");
        model->setItem(1,0,new QStandardItem("FVCC")); tmpslist.append("U[mV]");
        model->setItem(2,0,new QStandardItem("XXXX")); tmpslist.append("I[mA]");
        model->setItem(3,0,new QStandardItem("Mode")); tmpslist.append("Mode");
        model->setItem(4,0,new QStandardItem("DAdr")); tmpslist.append("DmxAddr");
        model->setItem(5,0,new QStandardItem("KDMX")); tmpslist.append("DmxValue");
        model->setItem(6,0,new QStandardItem("DIP_")); tmpslist.append("DIP");
        model->setItem(7,0,new QStandardItem("SWID")); tmpslist.append("SWID");
        model->setItem(8,0,new QStandardItem("SWV_")); tmpslist.append("SWV");
        model->setVerticalHeaderLabels(tmpslist);
        // NAME HW__ HWID HWV_ SW__ SWID SWV_  Mode Vmin AVol VCC_ FVCC


        ui.displayValues->setModel(model);


        sPort=settings.value("XBeePort","").toString();
        if(sPort.length()>0){
            qDebug()<<"OpenXBeePort " << sPort;
            ui.mycombox->OpenCom(sPort);
        }

        for(iFor=1;iFor<=15;iFor++){
            this->knownList.append(QString().sprintf("%04X",iFor));
        }
        ui.displayAutoFirm->setVisible(false);
}

void XBeeGui::DiplayDisplayValues(){
    QAbstractItemModel *tm = ui.displayValues->model();
    int iFor,nFor;
    nFor=tm->rowCount();
    for(iFor=0;iFor<nFor;iFor++){

    }
}
XBeeGui::eeMainstate XBeeGui::GetMainState(){
    return pMainstate;
}

void XBeeGui::ChangeMainstate(eeMainstate newState){
    bool bEnableNormal;
    bool bEnableBootloader;
    switch(newState){
    case eMsNormal:
        bEnableNormal    =true;
        bEnableBootloader=true;
        break;
    case eMsBootloader:
        bEnableNormal    =false;
        bEnableBootloader=true;
        break;
    case eMsAutoFlash:
        bEnableNormal    =false;
        bEnableBootloader=false;
        break;
    default:
        qDebug() << "forgotten case ChangeMainstate ";
        return;
    }
    pMainstate=newState;

    ui.checkSendDmx->setEnabled(bEnableNormal);
    ui.chkAutoFirmware->setEnabled(bEnableNormal);
    ui.chkKgetTimer->setEnabled(bEnableNormal);
    ui.listDevices->setEnabled(bEnableNormal);
    ui.mycombox->setEnabled(bEnableNormal);
    ui.pbKpngBroadcast->setEnabled(bEnableNormal);
    ui.pbKpngKnown->setEnabled(bEnableNormal);
    ui.pbTest->setEnabled(bEnableNormal);
    ui.pbTestRead->setEnabled(bEnableNormal);
    ui.pbTestWrite->setEnabled(bEnableNormal);

    //ui.pbKBoot->setEnabled(bEnableNormal);
    ui.pbKbootLogout->setEnabled(bEnableBootloader);
    ui.pbBootChipReset->setEnabled(bEnableBootloader);
    ui.pbBootFlashProgram->setEnabled(bEnableBootloader);
    ui.pbBootLoadFlash->setEnabled(bEnableBootloader);
    //ui.pbBootRequestCrc->setEnabled(bEnableBootloader);
    ui.pbBootTest->setEnabled(bEnableBootloader);

}

void XBeeGui::handleXbeeTx(QByteArray packet){
    QString s;
    if(this->port->isOpen()){
        this->port->write(packet);
        s.append(">>");
        s.append(packet.toHex());
        ui.textCom->append(s);
    }
}

void XBeeGui::handleXBeeRx(QByteArray packet){
    QString s;
    QByteArray data;
    QString sKeyAddr;
    QString sCmd;
    char PacketKnown=0;
    //qDebug("handleXBeeRx");
    s.append("<<");
#if 1
    s.append(packet.toHex());
#else
    long k;
    for(k=0;k<packet.length();k++){
            s.append(packet.at(k).toHex());
            s.append(" ");
    }
#endif
    ui.textCom->append(s);
    //ui.textCom->insert
    const uint8_t *p=(const uint8_t *)packet.constData();
    switch(*p){
    case xApiRecievePacket16:
        uint16_t addr;
        uint8_t rssi;
        uint8_t options;
        uint16_t kFrame;
        quint32 tmpu32;
        addr=((((uint16_t)p[1])<<8)+p[2]);
        rssi=p[3];
        options=p[4];
        data=packet.mid(5);
        //qDebug() << "xApiRecievePacket16 from " << addr << " rssi:" << rssi << "option:" << options << " - " << packet.toHex();
        sKeyAddr=QString().sprintf("%04X",addr);
        kDevice* mydevice;
        if(this->kDevices.contains(sKeyAddr)){
            mydevice=this->kDevices.value(sKeyAddr);
        }else{
            qDebug() << " device new";
            mydevice=new kDevice;
            mydevice->addr16=addr;
            this->kDevices.insert(sKeyAddr,mydevice);
            ui.listDevices->addItem(sKeyAddr);
            ui.listDevices->sortItems();

            if(ui.listDevices->count()==1){
                qDebug() << " first device";
                ui.listDevices->setCurrentItem(ui.listDevices->findItems(sKeyAddr,0).value(0));
            }
        }
        mydevice->lastreceived=QTime().currentTime();

        p=(const uint8_t *)data.constData();
        kFrame=((((uint16_t)p[1])<<8)+p[2]);
        data=data.mid(2);
        sCmd=data.left(4);

        (void) kFrame;
        (void) data;
        (void) sCmd;
        PacketKnown=0;
        if(sCmd.compare("xBOO")==0){
            myXbee.kFrameState[mykboot.kframe]=CXBee::eKFS_Fail;
            PacketKnown=1;
            //qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << "xBoo";
        }else if(sCmd.compare("kBOO")==0){
            myXbee.kFrameState[mykboot.kframe]=CXBee::eKFS_Success;
            //qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << "kBoo";
            switch(data.at(5)){
            case 'C':
                tmpu32=*((quint32*)data.mid(6,4).constData());
                qDebug() << "kBOOT("<<addr<<") Flash CRC is " << QString().sprintf("0x%08X",tmpu32);
                mydevice->flashcrc32=tmpu32;
                mykboot.receivedFlashCRC=tmpu32;
                if(GetSelectedXBeeAddr_uint16()==mydevice->addr16){
                    ui.labelProgCrc->setText(QString().sprintf("0x%08X",tmpu32));
                }
                PacketKnown=1;
                break;
            case 'f':
                tmpu32=*((quint32*)data.mid(6,4).constData());
                qDebug() << "kBOOT("<<addr<<") Flash read at " << QString().sprintf("0x%08X",tmpu32) << " : " << data.mid(6).toHex();
                qDebug() << "TODO kboot flashread";
                break;
            case 'F':
                tmpu32=*((quint32*)data.mid(6,4).constData());
                qDebug() << "kBOOT("<<addr<<") Flash Programmed at " << QString().sprintf("0x%08X",tmpu32);
                PacketKnown=1;
                break;
            case 'R':
                qDebug() << "kBOOT("<<addr<<") Chip reset";
                PacketKnown=1;
                break;
            case '1':
                qDebug() << "kBOOT("<<addr<<") Boot Logon";
                if(this->GetSelectedXBeeAddr_uint16()==addr){
                    switch(GetMainState()){
                    case eMsAutoFlash:
                        if(addr==mykboot.xbeeaddr){
                            mykboot.state=eFPS_FlashReset;
                        }else{
                            qDebug()<<"strange, BootLogin for " << addr << "received, but looking for " << mykboot.xbeeaddr;
                        }
                        break;
                    case eMsBootloader:
                        if(addr==mykboot.xbeeaddr){
                            qDebug()<<"another BootLogin" ;
                        }else{
                            qDebug()<<"strange, BootLogin for " << addr << "received, but looking for " << mykboot.xbeeaddr;
                        }
                        break;
                    case eMsNormal:
                        this->ChangeMainstate(eMsBootloader);
                        break;
                    default:
                        qDebug()<<"forgotten case HandleXbeeRxBootLogin" ;
                    }
                }else{
                    qDebug()<<"strange, BootLogin for " << addr << "received, while " << this->GetSelectedXBeeAddr_uint16() << "is selected";
                }
                PacketKnown=1;
                break;
            case '0':
                qDebug() << "kBOOT("<<addr<<") Boot Logoff";
                if(this->GetMainState()==eMsBootloader) this->ChangeMainstate(eMsNormal);
                PacketKnown=1;
                break;
            default:
                qDebug() << "xApiRecievePacket16 unknown KBOOT from " << addr << " - data " << data.toHex();
                break;
            }
        }else if(sCmd.compare("kGET")==0){
            QByteArray kgetkget=data.mid(0,4);
            QByteArray kgetid  =data.mid(4,4);
            QByteArray kgetdata=data.mid(8,-1);
            quint64 tmpint=kgetdata.mid(0,4).toHex().toUInt(0,16);
            qDebug() << "Recieve kGET(" <<  kgetid << ") from " << addr << " : (hex) " << kgetdata.toHex();
            qDebug() << "Recieve kGET(" <<  kgetid << ") from " << addr << " : (int) " << kgetdata.mid(0,4).toHex().toUInt(0,16);
            qDebug() << "Recieve kGET(" <<  kgetid << ") from " << addr << " : (bin) " << QString().setNum(tmpint, 2).prepend("0000000000").right(10);
            qDebug() << "Recieve kGET(" <<  kgetid << ") from " << addr << " : (raw) " << " - " << kgetdata;
            qDebug() << "dummy";

            mydevice->values[kgetid]=kgetdata;
            mydevice->valueReceiveTimes[kgetid]=QTime().currentTime();

        }
        if(PacketKnown==0){
            qDebug() << "xApiRecievePacket16 from " << addr << " rssi:" << rssi << "option:" << options << " - " << packet.toHex() << " data " << data.toHex();
        }
        //ui.listDevices->addColumn("Something");
        break;
    case xApiModemStatus:
        data=packet.mid(1);
        qDebug() << "xApiModemStatus " << data.toHex();
        break;
    case xApiTransmitStatus:
        data=packet.mid(1);
        //qDebug() << "xApiTransmitStatus" << data.toHex() ;
        //qDebug() << QTime().currentTime().toString("hh:mm:ss.zzz") << "xApiTransmitStatus";
        testInt=1;
        break;
    default:
        qDebug() << "FORGOTTEN XBEE-ApiIdentifier";
        break;
    }
}
long iTimer=0;
long iKbootTimeout=0;
long iDmxTx=0;
void XBeeGui::handleT1(){
    QString s;
    iTimer++;

#if 0
    uint8_t rec[150];
    DWORD i,k;
    if(myXbee.IsOpen()){
        myXbee.PortRead((char*)rec,sizeof(rec),&i);
        if(i>0){
            iCharsRx+=i;
            s.fromAscii("\n");
            for(k=0;k<i;k++){
                    s.append(st.number(rec[k],16));
                    s.append(" ");
            }
            ui.textCom->append(s);
            s.sprintf("%li bytes", iCharsRx);
            this->ui.labelComRx->setText(s);
        }
    }
#else
    if(port->bytesAvailable()>0){
        QByteArray d=port->readAll();
        myXbee.XBeeHandleData(d);
        s.sprintf("%li bytes", myXbee.iCharsRx);
        this->ui.labelComRx->setText(s);
    }
#endif

    // DMX send
    if((iTimer%20==0)&&(ui.checkSendDmx->isChecked())){
        if(pDmxData){
            myXbee.XBeeSend_KDMX(pDmxData,SENDKDMX_NUMBEROFVALUES);
            ui.labelDmxValues->setText(QString("Dmx: ").append( QByteArray((char*)pDmxData,10).toHex()));
            iDmxTx++;
            //uint8_t tmp;
            //tmp=myXbee.XBeeSend_KDMX(pDmxData,40);
            //qDebug() << "XBeeSend_KDMX " << tmp;
        }
        //myXbee.Test();
    }

    // send
    const int DbgRx_framerate=10;
    if((iTimer%(1000/DbgRx_framerate)==0)&&(ui.checkSendDbgRx->isChecked())){
        static int DbgRx_frame=0;
        static char DbgRx_msg[14]="xxKDBR";
        DbgRx_frame++;
        DbgRx_msg[0]='x';
        DbgRx_msg[1]='x';
        DbgRx_msg[2]='K';
        DbgRx_msg[3]='D';
        DbgRx_msg[4]='B';
        DbgRx_msg[5]='R';
        DbgRx_msg[6]=(DbgRx_framerate>>(3*8))&0xFF;
        DbgRx_msg[7]=(DbgRx_framerate>>(2*8))&0xFF;
        DbgRx_msg[8]=(DbgRx_framerate>>(1*8))&0xFF;
        DbgRx_msg[9]=(DbgRx_framerate>>(0*8))&0xFF;
        DbgRx_msg[10]=(DbgRx_frame>>(3*8))&0xFF;
        DbgRx_msg[11]=(DbgRx_frame>>(2*8))&0xFF;
        DbgRx_msg[12]=(DbgRx_frame>>(1*8))&0xFF;
        DbgRx_msg[13]=(DbgRx_frame>>(0*8))&0xFF;
        myXbee.XBeeSendPacket16(0xFFFF,(uint8_t*)DbgRx_msg,sizeof(DbgRx_msg),0,0,1);
        //myXbee.XBeeSend_KDMX()
    }



    if(mykboot.state){
        if(myXbee.kFrameState[mykboot.kframe]!=CXBee::eKFS_Used){
            qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " SendFlashProgramPacket x";
            SendFlashProgramPacket();
            qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " done";
        }else{
            iKbootTimeout++;
            if(iKbootTimeout>200){
                qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " resend lastSentPackage";
                this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
                qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " done";
                iKbootTimeout=0;
            }
        }
        if((iTimer%5==0)&&(mykboot.state==eFPS_LogInNecessary)){
            qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " SendFlashProgramPacket";
            SendFlashProgramPacket();
            qDebug() << "[BOOT] " << QTime().currentTime().toString(TimestampFormat) << " done";
        }
    }
}
void XBeeGui::handleT2(){
    QString s;
    s.sprintf("%5li Hz Rx-Timer\n%5li Hz Tx-Dmx\n%5li Hz Rx-Xbee\n%5li Hz Tx-XBee", iTimer, iDmxTx, this->myXbee.cntRxPackets , this->myXbee.cntTxPackets);
    this->myXbee.cntRxPackets=0;
    this->myXbee.cntTxPackets=0;
    this->ui.labelComTx->setText(s);
    iTimer=0;
    iDmxTx=0;
}


void XBeeGui::handlePbTestWrite(){
        qDebug() << "handlePbTestWrite ";
        if(this->port->isOpen()){
                //myXbee.PortWrite(c,sizeof(c));
                qDebug() << "write XBee...";
                //myXbee.XBeeSendAtCommand(xAtCmdSerial_Number_low);
                myXbee.Test();
        }else{
                qDebug("port is closed");
        }
}
void XBeeGui::handlePbTestRead(){
        qDebug() << "handlePbTestRead ";

        uint8_t rec[150];
        unsigned long i;
        unsigned int k;

        if(this->port->isOpen()){

                i = this->port->read((char*)rec,sizeof(rec));
                qDebug("Test read %lu chars.",i);
                QString s,st;
                for(k=0;k<i;k++){
                        s.append(st.number(rec[k],16));
                        s.append(" ");
                }
                qDebug() << s;
        }else{
                qDebug("port is closed");
        }
}

void XBeeGui::on_pbKpngBroadcast_clicked()
{
    this->myXbee.XBeeSendPing16(0xFFFF);
}

void XBeeGui::on_pbKpngKnown_clicked()
{
    uint16_t xbeeID;
    for(xbeeID=0x000;xbeeID<=0x0010;xbeeID++){
        this->myXbee.XBeeSendPing16(xbeeID);
    }
}

void XBeeGui::UpdateVersionList(){
    int iFor;
    QString sDeviceKey;
    int iDevice;
    QList <QString> keyList=this->kDevices.keys();
    kDevice* cDevice;

    ui.displayAutoFirm->clear();
    for(iFor=0;iFor<this->kDevices.size();iFor++){
        sDeviceKey=keyList.value(iFor);
        cDevice=this->kDevices.value(sDeviceKey);

        int tmpH,tmpL;
        QString tmpSWID=cDevice->values.value("SWID","????").toString();
        QByteArray tmpSWV=cDevice->values.value("SWV_","\0k\0k").toByteArray();
        tmpH=tmpSWV.mid(0,2).toHex().toUInt(0,16);
        tmpL=tmpSWV.mid(2,2).toHex().toUInt(0,16);
        QString tmpVer=QString().sprintf("%02x.%02x",tmpH,tmpL);
        ui.displayAutoFirm->addItem(sDeviceKey + " - " + QString().sprintf("0x%08X",cDevice->flashcrc32) + " - " + tmpSWID + " - " + tmpVer) ;
    }
    ui.displayAutoFirm->addItem("zz " + QTime().currentTime().toString());
    ui.displayAutoFirm->sortItems();
}

void XBeeGui::handleT3(){
    int iFor;

    //Auto-Check-Firmware
    if((this->GetMainState()==eMsNormal)&&(ui.chkAutoFirmware->isChecked()) && (myflash!=NULL) && (ui.checkSendDmx->isChecked()==false)){
        QString sDeviceKey;
        int iDevice;
        QList <QString> keyList=this->kDevices.keys();
        kDevice* cDevice;
        static int iDeviceShift=-1; // will be zero after first increment
        iDeviceShift++;
        if(iDeviceShift<(this->kDevices.size() + 0)){
            //just do that device [0 to size-1]
        }else if(iDeviceShift<(this->kDevices.size() + this->knownList.count())){
            //send a ping to known list [0 to size-1]
            int tmp=iDeviceShift-this->kDevices.size();
            if(tmp<0) tmp=0;
            if(tmp>(this->knownList.count()-1)) tmp=0;
            //this->myXbee.XBeeSendPing16(knownList.value(tmp).toUInt(0,16));
            this->myXbee.XBeeSendKcmd(knownList.value(tmp).toUInt(0,16),QByteArray("KBOOTC"));
            return;
        }else{
            iDeviceShift=0;

            UpdateVersionList();
        }
        for(iFor=0;iFor<this->kDevices.size();iFor++){
            iDevice=(iFor+iDeviceShift)%this->kDevices.size();
            sDeviceKey=keyList.value(iDevice);
            cDevice=this->kDevices.value(sDeviceKey);
            //HWID HWV_ SWV_ SWID
            if(cDevice->values.contains("HWID")==false){
                this->myXbee.XBeeSendKcmd(cDevice->addr16,QByteArray("KGETHWID"));
                return;
            }else if(cDevice->values.contains("HWV_")==false){
                this->myXbee.XBeeSendKcmd(cDevice->addr16,QByteArray("KGETHWV_"));
                return;
            }else if(cDevice->values.contains("SWID")==false){
                this->myXbee.XBeeSendKcmd(cDevice->addr16,QByteArray("KGETSWID"));
                return;
            }else if(cDevice->values.contains("SWV_")==false){
                this->myXbee.XBeeSendKcmd(cDevice->addr16,QByteArray("KGETSWV_"));
                return;
            }else if(cDevice->flashcrc32==0){
                this->myXbee.XBeeSendKcmd(cDevice->addr16,QByteArray("KBOOTC"));
                return;
            }else if(cDevice->flashcrc32!=myflash->GetCrc()){
                qDebug() << "start firmwareupdate " << sDeviceKey;

                this->SetSelectedXBee(sDeviceKey);
                this->ChangeMainstate(eMsAutoFlash);

                mykboot.xbeeaddr=cDevice->addr16;
                mykboot.progress=0;
                mykboot.state=eFPS_LogInNecessary;
                mykboot.retries=0;
                ui.pbarKBoot->setMaximum(myflash->size);
                this->SendFlashProgramPacket();
                return;
            }
        }
    }

    //Auto-Update DIP
    if(ui.chkKgetTimer->isChecked()){
        uint16_t iXBeeID=GetSelectedXBeeAddr_uint16();
        if(iXBeeID==0){
            qDebug() << "no XBee selected";
            return;
        }
        char sKget[9]="KGETDIP_";
        QByteArray kcmd(sKget,8);
        this->myXbee.XBeeSendKcmd(iXBeeID,kcmd);
    }
}

void XBeeGui::on_displayValues_clicked(const QModelIndex &index)
{
    uint16_t iXBeeID=GetSelectedXBeeAddr_uint16();
    if(iXBeeID==0){
        qDebug() << "no XBee selected";
        return;
    }
    char sKget[9]="KGET";
    qDebug() << "TODO:send KGET (" << index.row() << "," << index.column() << ")" << index.model()->itemData(index);;
    QModelIndex indexKGET=index.sibling(index.row(),0);
    qDebug() << "TESTTTT:" << index.model()->itemData(indexKGET);
    qDebug() << "TESTTTT:" << index.model()->data(indexKGET);
    sprintf(&sKget[4],index.model()->data(indexKGET).toString().toLatin1().data());
    qDebug() << "XBeeSendKcmd(" << iXBeeID << "," << sKget << ")";
    QByteArray kcmd(sKget,8);
    this->myXbee.XBeeSendKcmd(iXBeeID,kcmd);
}

void XBeeGui::on_listDevices_itemSelectionChanged()
{
     qDebug() << "TODO:on_listDevices_itemSelectionChanged";
}

void XBeeGui::handleComOpened(){
    QString sPort=ui.mycombox->GetLastOpenedPort();
    qDebug() << "handleComOpened: " << sPort;
    settings.setValue("XBeePort",sPort);
}


void XBeeGui::on_pbKBoot_clicked()
{
    qDebug() << "KBoot checkin";
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    uint16_t iAddr=GetSelectedXBeeAddr_uint16();
    this->myXbee.XBeeSendKcmd(iAddr,"KBOOT1");
    ui.checkSendDmx->setChecked(false);
}

void XBeeGui::on_pbKbootLogout_clicked()
{
    qDebug() << "KBoot checkout";
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    uint16_t iAddr=GetSelectedXBeeAddr_uint16();
    this->myXbee.XBeeSendKcmd(iAddr,"KBOOT0");
}

void XBeeGui::on_pbBootTest_clicked()
{
    qDebug() << "KBoot test";
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    uint16_t iAddr=GetSelectedXBeeAddr_uint16();
    QByteArray d;
    d.append("KBOOTF");
    d.append((char)0x00); // address lsb first
    d.append((char)0x80);
    d.append((char)0x00);
    d.append((char)0x08);

    d.append((char)0xAA);
    d.append((char)0xBB);
    d.append((char)0xCC);
    d.append((char)0xDD);
    d.append((char)0xEE);
    d.append((char)0xFF);
    this->myXbee.XBeeSendKcmd(iAddr,d);
}

void XBeeGui::on_pbTest_clicked()
{
    int iFor;
    //qDebug() << "no tests at the moment";
    uint8_t data[]={'x','x','K','P','N','G'};
    QTime timestart;
    timestart.start();
    for(iFor=0;iFor<1000;iFor++){
        testInt=0;
        this->myXbee.XBeeSendPacket16(9,data,sizeof(data),0,0,0);
        qDebug() << QTime().currentTime().toString("hh:mm:ss.zzz") << "send";
        while(testInt==0){
            QCoreApplication::processEvents();
        }
    }
    //QTime timetest=QTime().currentTime().-timestart;
    qDebug() << timestart.elapsed();
}

void XBeeGui::on_pbBootChipReset_clicked()
{
    qDebug() << "KBoot ChipReset";
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    uint16_t iAddr=GetSelectedXBeeAddr_uint16();
    QByteArray d;
    d.append("KBOOTR");
    this->myXbee.XBeeSendKcmd(iAddr,d);
}
void XBeeGui::loadFlashHex(){

    //QString sFile="E:/Test/FloKugel_V3_03.hex";
    //QString sFile="E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/TrueProject/Debug/FloKugel_V3_03.elf.hex";
    QString sFile="E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/bin_archiv/FloKugel_V3_03.elf.hex";
    int i;

    if(myflash!=NULL){
        delete myflash;
        myflash=NULL;
    }


    Flashbuffer*x=new Flashbuffer(0x08008000, BOOTLOADER_MAINAPP_SIZE);

    qDebug() << "getOpenFileName open";
    sFile=QFileDialog::getOpenFileName(this,tr("Open Hex File"),sFile,"Intel Hex Files(*.hex)");
    qDebug() << "getOpenFileName done";
    if(sFile.isEmpty())return;

    QFile f(sFile);
    if(!f.exists()){
        qDebug() << "ffffile does not exist "<<sFile;
        return ;
    }

    i=x->readIntelHex(sFile);
    qDebug() << "readIntelHex returned "<<i;
    /*qDebug() << "\n\n\n\n\n\n\n\n\n";
    unsigned int u;
    for(u=0;u<x->size;u+=16){
        qDebug() << QString().sprintf("0x%08X: ",u+x->offset) << x->buffer.mid(u,16).toHex();
    }*/

    if(i==0){
        myflash=x;
        ui.labelLoadCrc->setText(QString().sprintf("0x%08X",myflash->GetCrc()));
    }
}
void XBeeGui::loadFlashHex(QString sFile){

    //QString sFile="E:/Test/FloKugel_V3_03.hex";
    //QString sFile="E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/TrueProject/Debug/FloKugel_V3_03.elf.hex";
    //QString sFile="E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/bin_archiv/FloKugel_V3_03.elf.hex";
    int i;

    if(myflash!=NULL){
        delete myflash;
        myflash=NULL;
    }


    Flashbuffer*x=new Flashbuffer(0x08008000,BOOTLOADER_MAINAPP_SIZE);

    QFile f1(sFile);
    if(!f1.exists()){
        qDebug() << "getOpenFileName open";
        sFile=QFileDialog::getOpenFileName(this,tr("Open Hex File"),sFile,"Intel Hex Files(*.hex)");
        qDebug() << "getOpenFileName done";
        if(sFile.isEmpty())return;
    }

    QFile f(sFile);
    if(!f.exists()){
        qDebug() << "ffffile does not exist "<<sFile;
        return ;
    }

    i=x->readIntelHex(sFile);
    qDebug() << "readIntelHex returned "<<i;
    /*qDebug() << "\n\n\n\n\n\n\n\n\n";
    unsigned int u;
    for(u=0;u<x->size;u+=16){
        qDebug() << QString().sprintf("0x%08X: ",u+x->offset) << x->buffer.mid(u,16).toHex();
    }*/

    if(i==0){
        myflash=x;
        ui.labelLoadCrc->setText(QString().sprintf("0x%08X",myflash->GetCrc()));
    }
}

void XBeeGui::on_pbBootLoadFlash_clicked()
{
    this->loadFlashHex();
}
void XBeeGui::on_pbBootLoadFlashDevelop_clicked()
{
    //this->loadFlashHex("E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/TrueProject/Debug/FloKugel_V3_03.elf.hex");
    this->loadFlashHex("E:/Allwork/Programmieren/Mikrocontroller/STM32/FloKugel/CooProject/FloKugel/Debug/bin/FloKugel_V3_03.elf.hex");

}

void XBeeGui::on_pbBootFlashProgram_clicked()
{
    if(myflash==NULL){
        qDebug() << "myflash is NULL";
        return;
    }

    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    mykboot.xbeeaddr=GetSelectedXBeeAddr_uint16();
    mykboot.progress=0;
    mykboot.state=eFPS_FlashReset;
    mykboot.retries=0;
    ui.pbarKBoot->setMaximum(myflash->size);
    //this->SendFlashProgramPacket();
}

#define KFLASHPACKETSIZE 64
void XBeeGui::SendFlashProgramPacket(){
    QByteArray d;
    QByteArray tmpdata;
    quint32 flashaddr=0;
    quint32 bufferaddr=0;
    //int k;
    //qDebug() << "SendFlashProgramPacket(" << mykboot.state << ")";
    switch(mykboot.state){
    case eFPS_LogInNecessary:
        mykboot.lastSentPackage=QByteArray("KBOOT1");
        this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
        iKbootTimeout=0;
        mykboot.state=eFPS_LogInNecessary;
        break;
    case eFPS_FlashReset:
        qDebug() << "send flash reset";
        mykboot.lastSentPackage=QByteArray("KBOOTR");
        mykboot.kframe=this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
        iKbootTimeout=0;
        delete this->remoteflash;
        this->remoteflash=new Flashbuffer(0x08008000, BOOTLOADER_MAINAPP_SIZE);
        mykboot.state=eFPS_FlashCRC_request;//eFPS_Flash;
        break;
    case eFPS_Flash:
        while(1){
            mykboot.progress+=KFLASHPACKETSIZE;
            if(mykboot.progress>myflash->size){
                mykboot.state=eFPS_FlashComplete;
                return;
            }
            bufferaddr=myflash->size - mykboot.progress;
            flashaddr=myflash->offset + bufferaddr;

            tmpdata=myflash->buffer.mid(bufferaddr,KFLASHPACKETSIZE);
            if(QString(tmpdata.toHex()).compare("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",Qt::CaseInsensitive)==0){
                // only 0xFF
                //qDebug() << QString().sprintf("flash    skip 0x%08X: ",flashaddr) << tmpdata.toHex();
                //return;
            }else{
                break;
            }
        }
        qDebug() << QString().sprintf("flash program 0x%08X: '%s'",flashaddr,tmpdata.toHex().constData());
        d.clear();
        d.append("KBOOTF");
        d.append(Byte0(flashaddr));
        d.append(Byte1(flashaddr));
        d.append(Byte2(flashaddr));
        d.append(Byte3(flashaddr));
        d.append(tmpdata);
        mykboot.lastSentPackage=d;
        mykboot.kframe=this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
        iKbootTimeout=0;
        remoteflash->buffer.replace(bufferaddr,KFLASHPACKETSIZE,tmpdata);
        if((bufferaddr%0x400)==0){
            mykboot.state=eFPS_FlashCRC_request;
        }
        break;
    case eFPS_FlashCRC_request:
        mykboot.state=eFPS_FlashCRC_compare;
        qDebug() << "flash crc intermediate";
        mykboot.receivedFlashCRC++;
        mykboot.lastSentPackage=QByteArray("KBOOTC");
        this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
        iKbootTimeout=0;
        break;
    case eFPS_FlashCRC_compare:
        if(((quint32)mykboot.receivedFlashCRC)==this->remoteflash->GetCrc()){
            qDebug() << "intermediate CRC match";
            mykboot.state=eFPS_Flash;
            return;
        }else{
            qDebug() <<  QString().sprintf("crc mismatch: pc: 0x%08X" , remoteflash->GetCrc());
            qDebug() <<  QString().sprintf("crc mismatch: mC: 0x%08X" , mykboot.receivedFlashCRC);
            qDebug() << "quick and dirty QnD handling for intermediate CRC mismatch yet";
            if(mykboot.retries<2){
                //retry
                mykboot.retries++;
                qDebug() << "flash retry nr. " << mykboot.retries;
                mykboot.progress=0;
                mykboot.state=eFPS_FlashReset;
            }else{
                //give up
                qDebug() << "flash surrender";
                mykboot.state=eFPS_None;
            }
            return;
        }
        break;

    case eFPS_FlashComplete:
        mykboot.state=eFPS_FinalCRC;
        qDebug() << "flash crc final";
        mykboot.lastSentPackage=QByteArray("KBOOTC");
        this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
        iKbootTimeout=0;
        break;
    case eFPS_FinalCRC:
        if(((quint32)mykboot.receivedFlashCRC)==this->myflash->GetCrc()){
            qDebug() << "flash success";
            mykboot.state=eFPS_None;
            if(this->GetMainState()==eMsAutoFlash){
                mykboot.lastSentPackage=QByteArray("KBOOT0");
                this->myXbee.XBeeSendKcmd(mykboot.xbeeaddr,mykboot.lastSentPackage);
                iKbootTimeout=0;
                this->ChangeMainstate(eMsNormal);
            }
            if(ui.chkLogoutAfterProgram->checkState()==Qt::Checked){
                on_pbKbootLogout_clicked();
            }
        }else{
            qDebug() <<  QString().sprintf("crc mismatch: pc: 0x%08X" , myflash->GetCrc());
            qDebug() <<  QString().sprintf("crc mismatch: mC: 0x%08X" , mykboot.receivedFlashCRC);
            if(mykboot.retries<2){
                //retry
                mykboot.retries++;
                qDebug() << "flash retry nr. " << mykboot.retries;
                mykboot.progress=0;
                mykboot.state=eFPS_FlashReset;
            }else{
                //give up
                qDebug() << "flash surrender";
                mykboot.state=eFPS_None;
            }
        }
        break;
    }
    ui.pbarKBoot->setValue(mykboot.progress);
}

quint16 XBeeGui::GetSelectedXBeeAddr_uint16()
{
    if(ui.listDevices->currentItem()==NULL){
        return 0;
    }
    QString sAddr=ui.listDevices->currentItem()->text();
    quint16 iAddr=sAddr.toLong(0,16);
    return iAddr;
}
QString XBeeGui::GetSelectedXBeeAddr_string()
{
    if(ui.listDevices->currentItem()==NULL){
        return QString("");
    }
    return ui.listDevices->currentItem()->text();
}
void XBeeGui::SetSelectedXBee(QString sKeyAddr){
    ui.listDevices->setCurrentItem(ui.listDevices->findItems(sKeyAddr,0).value(0));
}

void XBeeGui::on_pbBootRequestCrc_clicked()
{
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    QString sAddr=GetSelectedXBeeAddr_string();
    uint16_t iAddr=sAddr.toLong(0,16);
    this->myXbee.XBeeSendKcmd(iAddr,QByteArray("KBOOTC"));
}

void XBeeGui::on_chkAutoFirmware_stateChanged(int newstate)
{
    switch(newstate){
    case Qt::Unchecked:

        ui.displayValues->setVisible(true);
        ui.displayAutoFirm->setVisible(false);

        qDebug() << "TODO";
        break;
    case Qt::Checked:
        this->loadFlashHex();

        ui.displayValues->setVisible(false);
        ui.displayAutoFirm->setVisible(true);

        qDebug() << "TODO";
        break;
    case Qt::PartiallyChecked:
        qDebug() << "ShouldNeverHappen";
        break;
    }
}

void XBeeGui::on_pbDmxW_clicked()
{
    int k;
    for(k=0;k<(SENDKDMX_NUMBEROFVALUES-4);k+=5){
        pDmxData[k+0]=0xFF;
        pDmxData[k+1]=0xFF;
        pDmxData[k+2]=0xFF;
        pDmxData[k+3]=0xFF;
        pDmxData[k+4]=0;
    }
}

void XBeeGui::on_pbDmxR_clicked()
{
    int k;
    for(k=0;k<(SENDKDMX_NUMBEROFVALUES-4);k+=5){
        pDmxData[k+0]=0xFF;
        pDmxData[k+1]=0x00;
        pDmxData[k+2]=0x00;
        pDmxData[k+3]=0xFF;
        pDmxData[k+4]=0;
    }
}

void XBeeGui::on_pbDmxG_clicked()
{
    int k;
    for(k=0;k<(SENDKDMX_NUMBEROFVALUES-4);k+=5){
        pDmxData[k+0]=0x00;
        pDmxData[k+1]=0xFF;
        pDmxData[k+2]=0x00;
        pDmxData[k+3]=0xFF;
        pDmxData[k+4]=0;
    }
}

void XBeeGui::on_pbDmxB_clicked()
{
    int k;
    for(k=0;k<(SENDKDMX_NUMBEROFVALUES-4);k+=5){
        pDmxData[k+0]=0x00;
        pDmxData[k+1]=0x00;
        pDmxData[k+2]=0xFF;
        pDmxData[k+3]=0xFF;
        pDmxData[k+4]=0;
    }
}

void XBeeGui::on_pbDmxX_clicked()
{
    int k;
    for(k=0;k<(SENDKDMX_NUMBEROFVALUES-4);k+=5){
        pDmxData[k+0]=0;
        pDmxData[k+1]=0;
        pDmxData[k+2]=0;
        pDmxData[k+3]=0;
        pDmxData[k+4]=0;
    }
}

void XBeeGui::on_lineKSEND_returnPressed()
{
    if(GetSelectedXBeeAddr_uint16()==0){
        qDebug() << "no device selected";
        return;
    }
    uint16_t iAddr=GetSelectedXBeeAddr_uint16();
    this->myXbee.XBeeSendKcmd(iAddr,this->ui.lineKSEND->text().toLatin1());
    ui.checkSendDmx->setChecked(false);
}
