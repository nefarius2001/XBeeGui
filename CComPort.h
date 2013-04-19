/*
 * CComPort.h
 *
 *  Created on: 19.06.2012
 *      Author: Nefarius
 */

#ifndef CCOMPORT_H_
#define CCOMPORT_H_

#include <windows.h>
#include <QtCore>

class CComPort {

public:
    DCB          comSettings;          // Contains various port settings
    COMMTIMEOUTS CommTimeouts;
    HANDLE       hPort      ;  // Handle COM port
    char 		 readBuffer[512];
    char * 		 readPointer;

	CComPort();
	virtual ~CComPort();
	int PortOpen(int iPort);
	int PortOpenS(char * sPort);

	int SetSettings();
	void GetSettings();
	int IsOpen();
	void PortClose(void);
	void AvailablePorts(QList<QString> *);

	// returns -1 on failure, number of written bytes on success
	int PortWrite( char* OUTBUFFER,      // Outgoing data
			       DWORD   bytes_to_write // Number of bytes to write
				   );

	int PortRead(char* OUTBUFFER,      // Outgoing data
			DWORD   bytes_to_read, // Number of bytes to read
			DWORD * bytes_read  // Number of bytes read
			);
};

#endif /* CCOMPORT_H_ */
