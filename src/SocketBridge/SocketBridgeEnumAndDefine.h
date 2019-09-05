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

	enum eOpcode
	{
		eOpcode_ajax_A = 'A',
		eOpcode_event_E = 'E',
		eOpcode_request_idCode = 'I',
		eOpcode_identify_W = 'W',
		eOpcode_unknown = 0x00
	};

    enum eEventType
    {
        eEventType_selectionAvailabilityUpdated  = 'a',
        eEventType_selectionPricesUpdated  = 'b',
        eEventType_creditUpdated = 'c',
        eEventType_cpuMessage = 'd',

        eEventType_selectionRequestStatus = 'e',
        eEventType_startSelection = 'f',
        eEventType_stopSelection = 'g',

		eEventType_cpuStatus = 'h',
		eEventType_answer_to_idCodeRequest = 'i',


        eEventType_unknown = 0xff
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
		u8	apiVersion;
		u8	unused1;
		u8	unused2;
		u8	unused3;
		
		void	zero()												{ apiVersion = 0; unused1 = 0; unused2 = 0; unused3 = 0; }
		bool	operator== (const sSokBridgeClientVer &b) const		{ return (apiVersion==b.apiVersion && unused1==b.unused1 && unused2==b.unused2 && unused3==b.unused3); }
		bool	operator!= (const sSokBridgeClientVer &b) const		{ return (apiVersion != b.apiVersion || unused1 != b.unused1 || unused2 != b.unused2 || unused3 != b.unused3); }
	} SokBridgeClientVer;


} // namespace socketbridge


#endif // _SocketBridgeEnumAndDefine_h_

