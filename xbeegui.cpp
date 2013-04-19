#include "xbeegui.h"
//#include <QtSerialPort>
//#include <QtAddOnSerialPort/serialport.h>
//#include <QtAddOnSerialPort/serialportinfo.h>
#include <QtCore>
#include <QtDebug>
#include <stdio.h>
//#include "serialport.h"
#include <combox.h>

XBeeGui::XBeeGui(QWidget *parent)
    : QMainWindow(parent)
{
    QLibrary l;
    qDebug("XBeeGui");
        ui.setupUi(this);
        port=new QextSerialPort("COM3");
        connect(ui.pbTest, SIGNAL(released()) , this, SLOT(handlePbTest()));
        connect(ui.pbTestRead, SIGNAL(released()) , this, SLOT(handlePbTestRead()));
        connect(ui.pbTestWrite, SIGNAL(released()) , this, SLOT(handlePbTestWrite()));
        connect(ui.pbComRefresh, SIGNAL(released())     , this, SLOT(handlePbRefreshPorts()));
        connect(ui.lstPorts,     SIGNAL(doubleClicked(QModelIndex)), this, SLOT(handleLstPortsDblClick(QModelIndex )));
        connect(ui.lstPorts,     SIGNAL(clicked(QModelIndex ))      , this, SLOT(handleLstPortsClick(QModelIndex)));
        connect(&this->cPort,SIGNAL(XbeeMsgRx(XBeeApiMsg_t)),this,SLOT(handleXBeeRx(XBeeApiMsg_t)));
        //ui.lstPorts->clicked()
        connect(&this->t1, SIGNAL(timeout()), this, SLOT(handleT1()));
        connect(&this->t2, SIGNAL(timeout()), this, SLOT(handleT2()));
        this->t1.setSingleShot(false);
        this->t2.setSingleShot(false);
        this->t1.start(1);
        this->t2.start(1000);
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
}
void XBeeGui::handleXBeeRx(XBeeApiMsg_t msg){
    QString s,st;
    long k;
    qDebug("rx");
    s.fromLatin1("\n");
    for(k=0;k<msg.MsgLen;k++){
            s.append(st.number(msg.Api.b[k],16));
            s.append(" ");
    }
    ui.textCom->append(s);

}
long iTimer=0;
long iDmxTx=0;
void XBeeGui::handleT1(){
    QString s,st;
    iTimer++;

#if 0
    uint8_t rec[150];
    DWORD i,k;
    if(cPort.IsOpen()){
        cPort.PortRead((char*)rec,sizeof(rec),&i);
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
    cPort.XBeeRxPoll();
    s.sprintf("%li bytes", cPort.iCharsRx);
    this->ui.labelComRx->setText(s);
#endif
    if((iTimer%10==0)&&(ui.checkSendDmx->isChecked())){
        if(pDmxData){
            cPort.XBeeSend_KDMX(pDmxData,40);
            ui.labelDmxValues->setText(QString("Dmx: ").append( QByteArray((char*)pDmxData,10).toHex()));
            iDmxTx++;
            //uint8_t tmp;
            //tmp=cPort.XBeeSend_KDMX(pDmxData,40);
            //qDebug() << "XBeeSend_KDMX " << tmp;
        }
        //cPort.Test();
    }
}

void XBeeGui::handleT2(){
    QString s;
    s.sprintf("%5li Hz Rx-Timer\n%5li Hz Tx-Dmx", iTimer, iDmxTx);
    this->ui.labelComTx->setText(s);
    iTimer=0;
    iDmxTx=0;
}

XBeeGui::~XBeeGui()
{
	qDebug("~XBeeGui");
}

void XBeeGui::handlePbRefreshPorts(){
	qDebug("handlePbRefreshPorts");
	this->RefreshPorts();
}
void XBeeGui::RefreshPorts(){
	QList<QString> ports;
	int i;
	qDebug("RefreshPorts");
	if (this->cPort.IsOpen()){
		qDebug()<< "close cPort first";
		cPort.PortClose();
	}

	cPort.AvailablePorts(&ports);
	ui.lstPorts->clear();
	qDebug("RefreshPorts.length=%i",ports.length());
	for (i=0; i<ports.length();i++){
		qDebug() << "RefreshPorts.port[" << i << "]=" << ports.value(i);
		ui.lstPorts->addItem(ports.value(i));
	}
	ui.lstPorts->setEnabled(true);
}
void XBeeGui::handleLstPortsDblClick(const QModelIndex & item ){
    QString sTmp;
	sTmp=ui.lstPorts->selectedItems().value(0)->text();
	qDebug() << "handlelstPortsDblClick: " << sTmp;
}
void XBeeGui::handleLstPortsClick(const QModelIndex & item ){
	QString sTmp;
	sTmp=ui.lstPorts->selectedItems().value(0)->text();
	qDebug() << "handlelstPortsClick: " << sTmp;
	if (this->cPort.IsOpen()){
		qDebug()<< "close cPort  (should never happen)";
		cPort.PortClose();
	}

	qDebug()<< "open cPort " << sTmp;
	sTmp.prepend("\\\\.\\");
	sTmp.append("\0");
    cPort.PortOpenS(sTmp.toLatin1().data());
	qDebug()<< "open cPort " << cPort.IsOpen();
	qDebug()<< "open cPort BaudRate " << cPort.comSettings.BaudRate;
	cPort.comSettings.BaudRate = 57600;
	cPort.comSettings.StopBits = ONESTOPBIT;
    cPort.comSettings.ByteSize = 8;
    cPort.comSettings.Parity   = NOPARITY;
    //cPort.comSettings.fParity  = FALSE;
    cPort.SetSettings();
	qDebug()<< "open cPort BaudRate " << cPort.comSettings.BaudRate;
    cPort.GetSettings();
	qDebug()<< "open cPort BaudRate " << cPort.comSettings.BaudRate;

	if(cPort.IsOpen()) ui.lstPorts->setEnabled(false);

}
#include <QString>
void XBeeGui::handlePbTest(){
	qDebug() << "handlePbTest ";

        //uint8_t rec[150];
        //DWORD i;
        //unsigned int k;

	if(cPort.IsOpen()){
		//cPort.PortWrite(c,sizeof(c));
                //qDebug() << "write XBee...";
		//cPort.XBeeSendAtCommand(xAtCmdSerial_Number_low);
                cPort.Test();
                cPort.XBeeSend_KTST();

                //cPort.PortRead((char*)rec,sizeof(rec),&i);
                //qDebug("Test read %lu chars.",i);
                //QString s,st;
                //for(k=0;k<i;k++){
                //	s.append(st.number(rec[k],16));
                //	s.append(" ");
                //}
                //qDebug() << s;
	}else{
		qDebug("cPort closed");
	}
}


void XBeeGui::handlePbTestWrite(){
        qDebug() << "handlePbTestWrite ";
        if(cPort.IsOpen()){
                //cPort.PortWrite(c,sizeof(c));
                qDebug() << "write XBee...";
                //cPort.XBeeSendAtCommand(xAtCmdSerial_Number_low);
                cPort.Test();
        }else{
                qDebug("cPort closed");
        }
}
void XBeeGui::handlePbTestRead(){
        qDebug() << "handlePbTestRead ";

        uint8_t rec[150];
        DWORD i;
        unsigned int k;

        if(cPort.IsOpen()){

                cPort.PortRead((char*)rec,sizeof(rec),&i);
                qDebug("Test read %lu chars.",i);
                QString s,st;
                for(k=0;k<i;k++){
                        s.append(st.number(rec[k],16));
                        s.append(" ");
                }
                qDebug() << s;
        }else{
                qDebug("cPort closed");
        }
}
