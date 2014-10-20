#include "xbeegui.h"

#include <QtGui>
#include <QApplication>
#include <qDebug>
#include <stdio.h>
#include <windows.h>
//#include <QtAddOnSerialPort/serialport.h>
//#include <QtAddOnSerialPort/serialportinfo.h>

#include "../qserialdevice/include/serialport.h"
#include "../qserialdevice/include/serialportinfo.h"

void test(void);
void test2(void);

int main(int argc, char *argv[])
{
    int ret;
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Chaos Company & Co");
    QCoreApplication::setApplicationName("kTerm");
    XBeeGui w;
    qDebug() << "main(): show XBeeGui now";
    w.show();
    //qDebug("test now");
    //test();
    //test2();
    qDebug() << "main(): execute application now";
    ret=a.exec();
    qDebug() << "main(): returning now";
    return ret;
}
/*
#include "CXBee.h"

#include <qDateTime>
//#include <iostream>
#include <windows.h>
#include <time.h>

using namespace std;

#define InBufSize 128

void test2(void){
SerialPortInfo iPort;
    // Example use SerialPortInfo
    foreach (const SerialPortInfo &info, SerialPortInfo::availablePorts()) {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        if (info.portName().compare("sdf")){
            iPort=info;
        }

    }

    // Example use SerialPort
    SerialPort s;
    s.setPort(iPort);
    if (s.open(QIODevice::ReadWrite)){
        s.write("kappppapasdf",12);
        s.close();
    }
    qDebug() << s.bytesToWrite();
}

void test(void)
{
    char INBUFFER[InBufSize+1];
    char OUTBUFFER[100]="The quick brown fox jumped over the lazy dog.\n";
    DWORD        bytes_read    = 0;    // Number of bytes read from port
    //DWORD        bytes_written = 0;    // Number of bytes written to the port
    CXBee cTestPort;
    int ret;


    // Open COM port
    ret=cTestPort.PortOpen(3);
    if (ret<0)
    {
    	qDebug() << "ERROR Com Open: " << ret << endl;
    	return;
    }
    // Set Port parameters.
    // Make a call to GetCommState() first in order to fill
    // the comSettings structure with all the necessary values.
    // Then change the ones you want and call SetCommState().
    cTestPort.comSettings.BaudRate = 57600;
    cTestPort.comSettings.StopBits = ONESTOPBIT;
    cTestPort.comSettings.ByteSize = 8;
    cTestPort.comSettings.Parity   = NOPARITY;
    cTestPort.comSettings.fParity  = FALSE;
    ret=cTestPort.SetSettings();
    if(ret<0)
    {
    	qDebug() << "ERROR SetCommState\n";
    }

	 QDateTime qtStart =QDateTime::currentDateTime();

		qDebug() << "write...";
		ret=cTestPort.PortWrite(OUTBUFFER,48);
	    if (ret< 0)
	    {
	    	qDebug("ERROR PortWrite returned %i\n",ret);
	    }

	    qDebug() << "done in " << qtStart.msecsTo(QDateTime::currentDateTime()) << " ms";

		qDebug() << "write XBee...";
		ret=cTestPort.XBeeSendAtCommand(xAtCmdSerial_Number_low);
	    if (ret< 0)
	    {
	    	qDebug("ERROR PortWrite returned %i\n",ret);
	    }

	    qDebug() << "done in " << qtStart.msecsTo(QDateTime::currentDateTime()) << " ms";


	 //time_t tStart;
	 //tStart = time (NULL);
	 qtStart =QDateTime::currentDateTime();
    do
    {
    	//qDebug("Read...");
    	ret=cTestPort.PortRead(INBUFFER,            // Incoming data
                (DWORD)InBufSize,                  // Number of bytes to read
                &bytes_read          // Number of bytes read)
                );
        if (ret< 0)
        {
        	qDebug("ERROR PortRead returned %i\n",ret);
        }
        INBUFFER[bytes_read]=0;
    	//qDebug("done\n");
        if (bytes_read){
        	qDebug() << "Receive after " << qtStart.msecsTo(QDateTime::currentDateTime()) << " ms";
        	qDebug("Bytes: %lu\n",bytes_read);
        	qDebug("'%s'\n",INBUFFER);
        }
        // code to do something with the data goes here
    } while(qtStart.msecsTo(QDateTime::currentDateTime()) < 3000);
    qDebug() << "listened for " << qtStart.msecsTo(QDateTime::currentDateTime()) << " ms";
    cTestPort.PortClose();

    qDebug() << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

}*/

