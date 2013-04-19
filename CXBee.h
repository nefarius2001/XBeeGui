/*
 * CXBee.h
 *
 *  Created on: 15.07.2012
 *      Author: Nefarius
 */

#ifndef CXBEE_H_
#define CXBEE_H_

#include "CComPort.h"

#define XBeeDataType			uint8_t
#include <stdint.h>

//---------------------
//------- Kaje --------
//---------------------

	#define	cPanIDKaje		0x4B56
	#define	cPanIDDefault	0x3332

	#define KRES	0x4B524553		//Reset for bootloader
	#define KDMX	0x4B444D58		//Dmx-Pakete
	#define KCMD	0x4B434D44
	#define KGET	0x4B474554
	#define kGET	0x6B474554
	#define xGET	0x78474554
	#define KSET	0x4B534554
	#define kSET	0x6B534554
		//#define cl_KDID	0x4b444944	//	Kaje Devide ID
		#define cl_NAME	0x4e414d45	//	Name
		#define cl_HWID 0x48574944	//  HardWare ID
		#define cl_HWV_	0x4857565f	//  HardWare Version
		#define cl_SWID	0x53574944	//  SoftWare ID
		#define cl_SWV_	0x5357565f	//  SoftWare Version
		#define cl_VCC_	0x5643435f	//  VCC in mV
		#define cl_FVCC	0x46564343	//  VCC in mV (nur von FloKugeln zu beantworten)
		#define cl_CC__	0x43435f5f	//  CurrentConsumption in mA
		#define cl_DAdr	0x44416472	//  Dmx-BasisAdresse
		#define cl_Mode	0x4D6F6465	//  Mode
		#define cl_BOTV	0x424F5456	//	BootloaderVersion
		#define cl_BOTR	0x424F5452	//	BootloaderReset
	#define KPNG	0x4B504E47
	#define KTST	0x4B545354

	#define cl_KBOO	0x4B424F4F	//	Bootloader Messages
		#define cl_FR	//Flash Read
		#define cl_FW	//Flash Write
		#define cl_ER	//Eeprom Read
		#define cl_EW	//Eeprom Write
		#define cl_LR	//Lockbits Read
		#define cl_SR	//Fusebits Read

//---------------------
//------- XBEE --------
//---------------------

	#define DisableAck_bm	(1<<1)
	#define BroadcastPanID_bm	(1<<3)

	#define XBeeMsgStart	0x7E

	#define	xApiModemStatus	0x8A            //RF module status messages are sent from the module in response to specific conditions
	#define	xApiAtCommand	0x08               //The “AT Command” API type allows for module parameters to be queried or set.
	#define	xApiAtCommandQueue	0x09          //Like “AT Command”, but values are not written until "AC"(Apply Changes) or "AT Command" is sent
	#define	xApiAtCommandResponse	0x88      //For Answers with more than one Frame. Answer is ended by an OK without data
	#define	xApiAtCommandRequest	0x17       //Allows for module parameter registers on a remote device to be queried or set
	#define	xApiRemoteCommandResponse	0x97  //If a module receives a remote command response RF data frame in response to a Remote AT Command Request, the module will send a Remote AT Command Response message out the UART.
	#define	xApiTransmitRequest64	0x00       //64-bit address A TX Request message will cause the module to send RF Data as an RF Packet.
	#define	xApiTransmitRequest16	0x01       //16-bit address A TX Request message will cause the module to send RF Data as an RF Packet.
	#define	xApiTransmitStatus	0x89       //When a TX Request is completed, the module sends a TX Status message. This message will indicate if the packet was transmitted successfully or if there was a failure.
	#define	xApiRecievePacket64	0x80       //64-bit When the module receives an RF packet, it is sent out the UART using this message type.
	#define	xApiRecievePacket16	0x81       //16-bit When the module receives an RF packet, it is sent out the UART using this message type.

	#define	xAtCmdEndDeviceAssociation	0x4131       //A1
	#define	xAtCmdCoordinatorAssociation	0x4132     //A2
	#define	xAtCmdApplyChanges	0x4143               //AC
	#define	xAtCmdAssociationIndication	0x4149      //AI
	#define	xAtCmdApiEnable	0x4150                  //AP
	#define	xAtCmdActiveScan	0x4153                 //AS
	#define	xAtCmdBaudRate	0x4244                   //BD
	#define	xAtCmdCcaThreshold	0x4341               //CA
	#define	xAtCmdCommand_Sequence_Character	0x4343 //CC
	#define	xAtCmdCoordinator_Enable	0x4345         //CE
	#define	xAtCmdChannel	0x4348                    //CH
	#define	xAtCmdExit_Command_Mode	0x434E          //CN
	#define	xAtCmdCommand_Mode_Timeout	0x4354       //CT
	#define	xAtCmdDIO0_Configuration	0x4430         //D0
	#define	xAtCmdDIO1_Configuration	0x4431         //D1
	#define	xAtCmdDIO2_Configuration	0x4432         //D2
	#define	xAtCmdDIO3_Configuration	0x4433         //D3
	#define	xAtCmdDIO4_Configuration	0x4434         //D4
	#define	xAtCmdDIO5_Configuration	0x4435         //D5
	#define	xAtCmdDIO6_Configuration	0x4436         //D6
	#define	xAtCmdDIO7_Configuration	0x4437         //D7
	#define	xAtCmdDI8_Configuration	0x4438          //D8
	#define	xAtCmdForce_Disassociation	0x4441       //DA
	#define	xAtCmdReceived_Signal_Strength	0x4442   //DB
	#define	xAtCmdDestination_Address_High	0x4448   //DH
	#define	xAtCmdDestination_Address_Low	0x444C    //DL
	#define	xAtCmdDestination_Node	0x444E           //DN
	#define	xAtCmdDisassociation_Cyclic_Sleep_Period	0x4450 //DP
	#define	xAtCmdACK_Failures	0x4541               //EA
	#define	xAtCmdCCA_Failures	0x4543               //EC
	#define	xAtCmdEnergy_Scan	0x4544                //ED
	#define	xAtCmdAES_Encryption_Enable	0x4545      //EE
	#define	xAtCmdForce_Poll	0x4650                 //FP
	#define	xAtCmdSoftware_Reset	0x4652             //FR
	#define	xAtCmdGuard_Times	0x4754                //GT
	#define	xAtCmdHardware_Version	0x4856           //HV
	#define	xAtCmdIO_Input_Address	0x4941           //IA
	#define	xAtCmdDIO_Change_Detect	0x4943          //IC
	#define	xAtCmdPan_ID	0x4944                     //ID
	#define	xAtCmdDigital_Output_Level	0x494F       //IO
	#define	xAtCmdSample_Rate	0x4952                //IR
	#define	xAtCmdForce_Sample	0x4953               //IS
	#define	xAtCmdSamples_before_TX	0x4954          //IT
	#define	xAtCmdIO_Output_Enable	0x4955           //IU
	#define	xAtCmdAES_Encryption_Key	0x4B59         //KY
	#define	xAtCmdPWM0_Output_Level	0x4D30          //M0
	#define	xAtCmdPWM1_Output_Level	0x4D31          //M1
	#define	xAtCmdMAC_Mode	0x4D4D                   //MM
	#define	xAtCmdSource_Address_16Bit	0x4D59       //MY
	#define	xAtCmdParity	0x4E42                     //NB
	#define	xAtCmdNode_Discover	0x4E44              //ND
	#define	xAtCmdNode_Identifier	0x4E49            //NI
	#define	xAtCmdNode_Discover_Options	0x4E4F      //NO
	#define	xAtCmdNode_Discover_Time	0x4E54         //NT
	#define	xAtCmdPWM0_Configuration	0x5030         //P0
	#define	xAtCmdPWM1_Configuration	0x5031         //P1
	#define	xAtCmdPower_Level	0x504C                //PL
	#define	xAtCmdPullup_Resistor	0x5052            //PR
	#define	xAtCmdPWM_Output_Timeout	0x5054         //PT
	#define	xAtCmdRestore_Defaults	0x5245           //RE
	#define	xAtCmdRandom_Delay_Slots	0x524E         //RN
	#define	xAtCmdPacketization_Timeout	0x524F      //RO
	#define	xAtCmdRSSI_PWM_Timer	0x5250             //RP
	#define	xAtCmdXBee_Retries	0x5252               //RR
	#define	xAtCmdScan_Channels	0x5343              //SC
	#define	xAtCmdScan_Duration	0x5344              //SD
	#define	xAtCmdSerial_Number_High	0x5348         //SH
	#define	xAtCmdSerial_Number_low	0x534C          //SL
	#define	xAtCmdSleep_Mode	0x534D                 //SM
	#define	xAtCmdSleep_Mode_Command	0x534F         //SO
	#define	xAtCmdCyclic_Sleep_Period	0x5350        //SP
	#define	xAtCmdTime_before_Sleep	0x5354          //ST
	#define	xAtCmdDIO0_Output_Timeout	0x5430        //T0
	#define	xAtCmdDIO1_Output_Timeout	0x5431        //T1
	#define	xAtCmdDIO2_Output_Timeout	0x5432        //T2
	#define	xAtCmdDIO3_Output_Timeout	0x5433        //T3
	#define	xAtCmdDIO4_Output_Timeout	0x5434        //T4
	#define	xAtCmdDIO5_Output_Timeout	0x5435        //T5
	#define	xAtCmdDIO6_Output_Timeout	0x5436        //T6
	#define	xAtCmdDIO7_Output_Timeout	0x5437        //T7
	#define	xAtCmdFirmware_Version_Verbose	0x564C   //VL
	#define	xAtCmdFirmware_Version	0x5652           //VR
	#define	xAtCmdeWrite	0x5752                     //WR



	#define kcMSBfirst(c1,c2,c3,c4)	(((qint32)c1<<(4*8))|((qint32)c2<<(2*8))|((qint32)c3<<(1*8))|((qint32)c4<<(0*8)))
	#define kcLSBfirst(c1,c2,c3,c4)	(((qint32)c1<<(0*8))|((qint32)c2<<(1*8))|((qint32)c3<<(2*8))|((qint32)c4<<(3*8)))
	#define kcToUint32(c1,c2,c3,c4)	kcLSBfirst(c1,c2,c3,c4)
	#define kcToUint16(c1,c2)	kcLSBfirst(c1,c2,0,0)



	enum XBeeMessageStatus{
		XBeeStateUnused=0,
		XBeeStateReserved,//=waiting for StartToken
		XBeeStateSizeMsb,
		XBeeStateSizeLsb,
		XBeeStateData,
		//XBeeStateCRC,
		XBeeStateValid,		//please now handle
		XBeeStateToBeSent,
		XBeeStateSending
	};
#define StateOnError XBeeStateUnused
    enum XBeeErrorCode{
        XErrToken=(1<<0),
        XErrSize=(1<<1),
        XErrCrc=(1<<2),
        XErrForgottenState=(1<<3)
    };

	typedef union {
		uint8_t b[110];
		uint8_t ApiIdentifier;
		struct{
			uint8_t ApiIdentifier;
			uint8_t Source_H;
			uint8_t Source_L;
			uint8_t RSSI;
			uint8_t Options;
			uint8_t RxData[100];
		}RX16;
		struct{
			uint8_t ApiIdentifier;
			uint8_t FrameID;
			uint8_t Destination_H;
			uint8_t Destination_L;
			uint8_t Options;
			uint8_t TxData[100];
		}TX16;
		struct{
			uint8_t ApiIdentifier;
			uint8_t Status;
		}ModemStatus;
	} XBeeApi_t;

	typedef struct {
		volatile int Status;
		quint16 MsgLen;
		quint16 Addr16;
		//uint16_t WritenBytes;
		uint8_t * pWrite;
		uint8_t * pCRC;
	    XBeeApi_t Api;

	}XBeeApiMsg_t;

	#define Rx16HeaderLen	5
	#define Tx16HeaderLen	5





class CXBee : public QObject,public CComPort
{
    Q_OBJECT
public:
	CXBee();
	virtual ~CXBee();


        long iCharsRx;

        XBeeDataType    GetFrameID();
        void 		XBeeSendApiPacket(int len, XBeeDataType *pData);
	XBeeDataType	XBeeSendAtCommand(uint16_t command);
	XBeeDataType	XBeeSetParameter(uint16_t command,qint32 value);
	XBeeDataType	XBeeSendPacket16(uint16_t addr, XBeeDataType *pData, int DataLen, char DisableAck, char BroadcastPanID, char NoResponse);
        void		XBeeSend_KTST(void);
        void 		Test(void);
        XBeeDataType    XBeeSend_KDMX(uint8_t *pDmxValues, unsigned char NrOfValues);
        void            XBeeRxPoll();
        void            XBeeHandleByte(XBeeDataType cIn);

signals:
        void            XbeeMsgError(int Errorcode);
        void            XbeeMsgRx(XBeeApiMsg_t msg);
};

#endif /* CXBEE_H_ */
