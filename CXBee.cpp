/*
 * CXBee.cpp
 *
 *  Created on: 15.07.2012
 *      Author: Nefarius
 */

#include "CXBee.h"

CXBee::CXBee() {
    iCharsRx=0;
	// TODO Auto-generated constructor stub

}

CXBee::~CXBee() {
	// TODO Auto-generated destructor stub
}


XBeeDataType CXBee::GetFrameID(){
static XBeeDataType k=0;
    k++;
    if (k<=0) k=1;
    return k;
}
	#define Byte3(xxx) ((xxx >>(8*3)) & 0xFF)
	#define Byte2(xxx) ((xxx >>(8*2)) & 0xFF)
	#define Byte1(xxx) ((xxx >>(8*1)) & 0xFF)
	#define Byte0(xxx) ((xxx >>(8*0)) & 0xFF)
	void CXBee::XBeeSendApiPacket(int len, XBeeDataType *pData){
	XBeeDataType mCRC;
        XBeeDataType tmp[3];
                tmp[0]=XBeeMsgStart;
                tmp[1]=Byte1(len);
                tmp[2]=Byte0(len);
                this->PortWrite((char*) tmp,3);

                this->PortWrite((char*) pData,len);
		mCRC=0xFF;
		while (len--){
			mCRC-=*pData;
			pData++;
		}
                this->PortWrite((char*) &mCRC,1);
	}

	XBeeDataType CXBee::XBeeSendAtCommand(uint16_t command){
	XBeeDataType FrameID;
	XBeeDataType XBeeSendData[4];
		FrameID = GetFrameID();
		XBeeSendData[0]=xApiAtCommand;
		XBeeSendData[1]=FrameID;
		XBeeSendData[2]=Byte1(command);
		XBeeSendData[3]=Byte0(command);
		XBeeSendApiPacket(4,XBeeSendData);
		return FrameID;
	}

	XBeeDataType CXBee::XBeeSetParameter(uint16_t command,qint32 value){
	XBeeDataType ThisApi[8];
		ThisApi[0]=xApiAtCommand;
		ThisApi[1]=GetFrameID();
		ThisApi[2]=Byte1(command);
		ThisApi[3]=Byte0(command);
		ThisApi[4]=Byte3(value);
		ThisApi[5]=Byte2(value);
		ThisApi[6]=Byte1(value);
		ThisApi[7]=Byte0(value);
		XBeeSendApiPacket(8,ThisApi);
		return ThisApi[1];
	}

	XBeeDataType CXBee::XBeeSendPacket16(uint16_t addr, XBeeDataType *pData, int DataLen, char DisableAck, char BroadcastPanID, char NoResponse){
	XBeeApiMsg_t TxMsg;
	XBeeDataType FrameID;
	XBeeDataType options;
	int p;

		if(DataLen>100){
			DataLen=100;
		}
		options=0;
		if(DisableAck)
			options |= (1<<1);
		if(BroadcastPanID)
			options |= (1<<3);
		if(NoResponse){
			FrameID=0;
		}else{
			FrameID=GetFrameID();
		}
		TxMsg.Api.b[0]=xApiTransmitRequest16;
		TxMsg.Api.b[1]=	FrameID;
		TxMsg.Api.b[2]=	Byte1(addr);
		TxMsg.Api.b[3]=	Byte0(addr);
		TxMsg.Api.b[4]=	options;
		for(p=0;p<DataLen;p++){
			TxMsg.Api.b[5+p]= *pData;
			pData++;
		}
		XBeeSendApiPacket(DataLen+5,&TxMsg.Api.b[0]);
		TxMsg.Status=XBeeStateUnused;
		return FrameID;
	}

        void CXBee::XBeeSend_KTST(void){
                uint8_t sTest[]="KTST";
                XBeeSendPacket16(0xFFFF, &sTest[0], 4, 0 , 0, 0);	//Response
                //XBeeSendPacket16(0xFFFF, sTest, 4, 1, 0, 1);	//NoResponse
        }
        XBeeDataType CXBee::XBeeSend_KDMX(uint8_t *pDmxValues, unsigned char NrOfValues){
                uint8_t sData[100]="xxKDMX";
                int k;
                if(NrOfValues>70) NrOfValues=70;
                sData[6]=0;
                sData[7]=1;
                for(k=0;k<NrOfValues;k++){
                    sData[8+k]=pDmxValues[k];
                }
                k--;
                //return XBeeSendPacket16(0xFFFF, &sData[0], 6+k, 0 , 0, 0);	//Response
                return XBeeSendPacket16(0xFFFF, &sData[0], 8+k, 0 , 0, 1);	//NoResponse
        }
	void CXBee::Test(void){
		uint8_t sTest[]= {0x7e, 0x00, 0x04, 0x08, 0x52, 0x4D, 0x59, 0xFF};
		this->PortWrite((char*)sTest,sizeof(sTest));

                XBeeSendApiPacket(4,&sTest[3]);
	}

        void  CXBee::XBeeRxPoll(){
            uint8_t rec[150];
            DWORD iBytesRead,k;
            if(this->IsOpen()){
                this->PortRead((char*)rec,sizeof(rec),&iBytesRead);
                iCharsRx+=iBytesRead;
                for(k=0;k<iBytesRead;k++){
                    this->XBeeHandleByte(rec[k]);
                }
            }
        }

    uint8_t XBeeErrorFlags;
    void CXBee::XBeeHandleByte(XBeeDataType cIn){
    //int8_t NewRxMsg;

    static XBeeApiMsg_t RxMsg123;
    XBeeApiMsg_t *pXRecMsg =&RxMsg123;
        /*static uint32_t xuptime=0;
        if(pXRecMsg->Status){// TODO: Timeout
            if(uptime>(xuptime+2)){
                //PrintfTimeout("Xtimeout\n");
                pXRecMsg->Status=0;
            }
        }
        xuptime = uptime;*/
        //if(pXRecMsg->Status>0){
            switch(pXRecMsg->Status){
            case XBeeStateUnused: //Start-Token
            case XBeeStateReserved:
                if(cIn==XBeeMsgStart){
                    //pXRecMsg->MsgLen=0;
                    pXRecMsg->pWrite=&pXRecMsg->Api.ApiIdentifier;
                    pXRecMsg->Status=XBeeStateSizeMsb;
                    //PrintfRxBytes("XToken\n");
                }else{
                    emit XbeeMsgError(XErrToken);
                    //PrintfWrongBytes("Xerr %c\n",cIn);
                }
                break;
            case XBeeStateSizeMsb://Size MSB
                pXRecMsg->MsgLen=(cIn<<8);
                pXRecMsg->Status=XBeeStateSizeLsb;
                break;
            case XBeeStateSizeLsb://Size LSB
                pXRecMsg->MsgLen += cIn;
                if((pXRecMsg->MsgLen <=0)|(pXRecMsg->MsgLen >= sizeof(pXRecMsg->Api.b))){
                    //error
                    //PrintfSizeError("Xerr Size:%i\n",pXRecMsg->MsgLen.i);
                    emit XbeeMsgError(XErrSize);
                    pXRecMsg->Status=StateOnError;
                }else{
                    // everythings ok
                    pXRecMsg->pCRC=&pXRecMsg->Api.b[pXRecMsg->MsgLen];	//Set CRC-Pointer
                    *(pXRecMsg->pCRC)=0;									//CRC=0;
                    //PrintfRxBytes("XSize:%i (cIn=%i)\n",pXRecMsg->MsgLen,cIn);
                    pXRecMsg->Status=XBeeStateData;							//next state
                }
                break;
            case XBeeStateData://Data
                *(pXRecMsg->pCRC)+=cIn;					//update CRC

                //PrintfRxBytes("%02x",cIn);
                //PrintfRxBytes("%c",cIn);
                if(pXRecMsg->pWrite < (pXRecMsg->pCRC)){	//if still data
                    *(pXRecMsg->pWrite++)=cIn;				//	save byte
                }else{										//if CRC-Byte (data complete)
                    //PrintfRxBytes("\n");

                    if(*(pXRecMsg->pCRC)==0xFF){
                        //PrintfRxBytes("X[PacketOK]\n");
                        pXRecMsg->Status=XBeeStateValid;
                        emit XbeeMsgRx(*pXRecMsg);
                        pXRecMsg->Status=XBeeStateUnused;
                    }else{
                        emit XbeeMsgError(XErrCrc);
                        //PrintfCrcError("Xerr[CRC] %i!=%i (%i)\n",*(pXRecMsg->pCRC),cIn,pXRecMsg->MsgLen.i);	//Checksum-Error
                        pXRecMsg->Status=StateOnError;
                    }
                }
                break;
            default:
                emit XbeeMsgError(XErrForgottenState);
                pXRecMsg->Status=StateOnError;
                break;
            }
        //}
    }
