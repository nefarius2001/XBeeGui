#include "kBridgeDll.h"

#include <QtDebug>
#include <QString>
#include <QMessageBox>

KBridgeDll::KBridgeDll()
{
    int iFor;
    for(iFor=0;iFor<DmxDataFallback_NUMEL;iFor++){
        DmxDataFallback[iFor]=0;
    }
    qDebug("Load kBridgeDll");
    DmxGetOutPointer=0;
    pDmxData=0;
    QString sdll= QString("kBridgeDll");
    //QString sdll=QString ( getenv("WINDIR") ) + "\\system32\\kBridgeDll.dll";
    libKDmxBridge=new QLibrary(sdll);
    if(libKDmxBridge->load()==false){
        qWarning("kBridgeDll NOT loaded");
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
    if(pDmxData==0){
        pDmxData=DmxDataFallback;
        QMessageBox msgBox;
        qWarning("Failed to get DmxOutPointer from kBridgeDll.dll, using local array DmxDataFallback[512] instead");
        msgBox.setText("Failed to get DmxOutPointer from kBridgeDll.dll, using local array DmxDataFallback[512] instead");
        msgBox.exec();
    }
}
