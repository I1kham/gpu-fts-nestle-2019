#ifndef _SocketBridgeEnumAndDefine_h_
#define _SocketBridgeEnumAndDefine_h_
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/rheaHandleUID88.h"

/* richieste dalla console al server
   Per convenzioni, il MSB deve essere 0x03*/
#define GUIBRIDGE_CONSOLE_EVENT_QUIT                0x0301
#define GUIBRIDGE_CONSOLE_EVENT_PING                0x0302
#define GUIBRIDGE_CONSOLE_EVENT_CLOSE               0x0303
#define GUIBRIDGE_CONSOLE_EVENT_STRING              0x0304
#define GUIBRIDGE_CONSOLE_EVENT_CLIENT_LIST         0x0305




namespace socketbridge
{
	class Server;

	enum class eOpcode: u8
	{
		ajax_A = 'A',
		event_E = 'E',
		request_idCode = 'I',
		identify_W = 'W',
		fileTransfer = 'F',
		unknown = 0x00
	};

    enum class eEventType: u8
    {
        selectionAvailabilityUpdated  = 'a',	//97	0x61
        selectionPricesUpdated  = 'b',			//98	0x62
        creditUpdated = 'c',					//99	0x63
        cpuMessage = 'd',						//100	0x64

        selectionRequestStatus = 'e',			//101	0x65
        startSelection = 'f',					//102	0x66
        stopSelection = 'g',					//103	0x67

		cpuStatus = 'h',						//104	0x68
		answer_to_idCodeRequest = 'i',			//105	0x69
		
		reqClientList ='j',						//106	0x6A
		btnProgPressed ='k',					//107	0x6B
		reqDataAudit = 'l',						//108	0x6C
		reqIniParam = 'm',						//109	0x6D
		reqVMCDataFile = 'n',					//110	0x6E
		reqVMCDataFileTimestamp = 'o',			//111	0x6F
		reqWriteLocalVMCDataFile = 'p',			//112	0x70
		cpuProgrammingCmd = 'q',				//113	0x71
		cpuSanWashingStatus = 'r',				//114	0x72
		cpuBtnPressed = 's',					//115	0x73
		cpuWritePartialVMCDataFile = 't',		//116	0x74
		cpuExtendedConfigInfo = 'u',			//117	0x75
		setDecounter = 'v',						//118	0x76
		getAllDecounters = 'w',					//119	0x77
		setMotoreMacina = 'x',					//120	0x78
		getAperturaVGrind = 'y',				//121	0x79
		setAperturaVGrind = 'z',				//122	0x80
		startDisinstallation = 'A',				//65	0x41
		recalcFasciaOrariaFV = 'B',				//66	0x42
		enterDA3Sync = 'C',						//67	0x43
		startSelectionAlreadyPaid = 'D',		//68	0x44
		AlipayChina_onlineStatusChanged = 'E',	//69	0x45
		startSelectionForceJug = 'F',			//70	0x46
        unknown = 0xff
    };

	struct sDecodedMessage
	{
		eOpcode	opcode;
		u8		requestID;
		u16		payloadLen;
		u8		*payload;
	};


	//handle per le gestione dei clienti di socketbridge::Server
	typedef struct sHandleSokBridgeClient
	{
		u16	index;

		bool	operator== (const sHandleSokBridgeClient &b) const					{ return this->index == b.index; }
		bool	operator!= (const sHandleSokBridgeClient &b) const					{ return this->index != b.index; }
	} HSokBridgeClient;


	typedef struct sSokBridgeIDCode
	{
		union uData
		{
			u32	asU32;
			u8	buffer[4];
		} data;

		bool	operator== (const sSokBridgeIDCode &b) const						{ return (data.asU32 == b.data.asU32); }
		bool	operator!= (const sSokBridgeIDCode &b) const						{ return (data.asU32 != b.data.asU32); }
	} SokBridgeIDCode;


	typedef struct sSokBridgeClientVer
	{
		static const u8 APP_TYPE_GUI = 0x01;
		static const u8 APP_TYPE_CONSOLE = 0x02;
		
		u8	apiVersion;
		u8	appType;
		u8	unused2;
		u8	unused3;
		
		void	zero()												{ apiVersion = 0; appType = 0; unused2 = 0; unused3 = 0; }
		bool	operator== (const sSokBridgeClientVer &b) const		{ return (apiVersion==b.apiVersion && appType ==b.appType && unused2==b.unused2 && unused3==b.unused3); }
		bool	operator!= (const sSokBridgeClientVer &b) const		{ return (apiVersion != b.apiVersion || appType != b.appType || unused2 != b.unused2 || unused3 != b.unused3); }
	} SokBridgeClientVer;


	struct sIdentifiedClientInfo
	{
		HSokBridgeClient	handle;
		u32					currentWebSocketHandleAsU32;		//HSokServerClient  (invalid se la connessione al momento è chiusa)

		SokBridgeIDCode		idCode;								//codice univoco di identificazione
		SokBridgeClientVer	clientVer;

		u64     timeCreatedMSec;								//timestamp della creazione del record
		u64     lastTimeRcvMSec;								//timestamp dell'ultima volta che ho ricevuto dei dati
	};

} // namespace socketbridge


#endif // _SocketBridgeEnumAndDefine_h_

