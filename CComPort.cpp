/*
 * CComPort.cpp
 *
 *  Created on: 19.06.2012
 *      Author: Nefarius
 */

#include "CComPort.h"
#include "stdio.h"
#include <QDebug>

CComPort::CComPort() {
	hPort = INVALID_HANDLE_VALUE;
	readPointer=&readBuffer[0];
}

CComPort::~CComPort() {
	PortClose();
}
int CComPort::IsOpen() {
	return (hPort != INVALID_HANDLE_VALUE);
}

int CComPort::PortOpen(int iPort) {
	char sTmp[]="\\\\.\\COMX";
	sTmp[7]='0'+iPort;
	return PortOpenS(sTmp);
}

int CComPort::PortOpenS(char sCom[]) {
	int bStatus;
	qDebug("open com '%s'",sCom);
	hPort = CreateFileA(sCom,                // open com7:
	                    GENERIC_READ | GENERIC_WRITE, // for reading and writing
	                    0,                            // exclusive access
	                    NULL,                         // no security attributes
	                    OPEN_EXISTING,
	                    FILE_ATTRIBUTE_NORMAL,
	                    NULL);
	if (hPort == INVALID_HANDLE_VALUE){
		return -1; //failed
	}else{
		// Set timeouts in milliseconds
		CommTimeouts.ReadIntervalTimeout         = MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
		CommTimeouts.ReadTotalTimeoutConstant    = 0;
		CommTimeouts.WriteTotalTimeoutMultiplier = 0;
		CommTimeouts.WriteTotalTimeoutConstant   = 100;
		bStatus = SetCommTimeouts(hPort,&CommTimeouts);
	    if (bStatus == 0)
	    {
	    	return -2;	// Error setting Timeouts
	    }else{
	        GetCommState(hPort, &comSettings);

	    	this->comSettings.BaudRate = 57600;
	    	this->comSettings.StopBits = ONESTOPBIT;
	    	this->comSettings.ByteSize = 8;
	    	this->comSettings.Parity   = NOPARITY;
	    	this->comSettings.fParity  = FALSE;
	    	//this->comSettings.fDtrControl = DTR_CONTROL_ENABLE;
	    	//this->comSettings.fRtsControl = RTS_CONTROL_ENABLE;
	    	//this->comSettings.fDtrControl = DTR_CONTROL_DISABLE;
	    	//this->comSettings.fRtsControl = RTS_CONTROL_DISABLE;
	    	return this->SetSettings(); // success
	    }
	}
}
int CComPort::SetSettings(){
    int bStatus = SetCommState(hPort, &comSettings);
    if (bStatus == 0){
    	return -1;
    }else{
    	return 0; //success
    }
}
void CComPort::GetSettings(){
	GetCommState(hPort, &comSettings);
	return;
}

void CComPort::PortClose() {
    CloseHandle(hPort);
    hPort=INVALID_HANDLE_VALUE;
}

int CComPort::PortWrite( 	char* OUTBUFFER,      // Outgoing data
							DWORD   bytes_to_write // Number of bytes to write
			   	   	   )
{
	int bStatus;
	DWORD bytes_written;
	bStatus = WriteFile(hPort,         // Handle
						OUTBUFFER,      // Outgoing data
						bytes_to_write,  // Number of bytes to write
						&bytes_written,  // Number of bytes written
						NULL);
	if (bStatus == 0)
	{
		return -1; //fail
	}else{
		return bytes_written; //success
	}
}
int CComPort::PortRead(char* INBUFFER,      // Outgoing data
		DWORD   bytes_to_read, // Number of bytes to read
		DWORD * bytes_read  // Number of bytes read
		)
{
	int bStatus;
    bStatus = ReadFile(hPort,   // Handle
            INBUFFER,            // Incoming data
            bytes_to_read,                  // Number of bytes to read
            bytes_read,          // Number of bytes read
            NULL);
    if (bStatus == 0)
    {
    	return -1;
    }else{
    	INBUFFER[*bytes_read]=0;
    	return 0;
    }
}





#include <stdio.h>
#include <string.h>
#include <windows.h>


void CComPort::AvailablePorts(QList<QString> * ports)
{
	int i;
  //Make sure we clear out any elements which may already be in the array(s)

  //What will be the return value from this function (assume the worst)
  //BOOL bSuccess = FALSE;

  HKEY hSERIALCOMM;
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hSERIALCOMM) == ERROR_SUCCESS)
  {
		//Get the max value name and max value lengths
		DWORD dwMaxValueNameLen;
		DWORD dwMaxValueLen;
		DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);
		if (dwQueryInfo == ERROR_SUCCESS)
		{
			DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the NULL terminator
			//DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars * sizeof(CHAR);
			DWORD dwMaxValueDataSizeInChars = dwMaxValueLen/sizeof(CHAR) + 1; //Include space for the NULL terminator
			DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars * sizeof(CHAR);

			//Allocate some space for the value name and value data
      CHAR  szValueName[5000]={0};
      BYTE   byValue[5000]={0};

		//Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
		DWORD dwIndex = 0;
		DWORD dwType;
		DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
		DWORD dwDataSize = dwMaxValueDataSizeInBytes;
		LONG nEnum = RegEnumValueA(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);
		while (nEnum == ERROR_SUCCESS)
		{
			//If the value is of the correct type, then add it to the array
			if (dwType == REG_SZ)
			{
				//TCHAR* szPort = reinterpret_cast<TCHAR*>(byValue);
				qDebug("coms=%s",byValue);
                ports->append(QString::fromLocal8Bit((char*)byValue,dwDataSize-1));
			}

			//Prepare for the next time around
			dwValueNameSize = dwMaxValueNameSizeInChars;
			dwDataSize = dwMaxValueDataSizeInBytes;
			for(i=0;i<5000;i++){
				szValueName[i]=0;
				byValue[i]=0;
			}
			++dwIndex;
			nEnum = RegEnumValueA(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);
		}
		}
		//Close the registry key now that we are finished with it
    RegCloseKey(hSERIALCOMM);

    if (dwQueryInfo != ERROR_SUCCESS)
			SetLastError(dwQueryInfo);
  }

	qDebug("ports.length=%i",ports->length());
	return;
}
