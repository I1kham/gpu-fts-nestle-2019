#ifndef _rheaAppEnumAndDefine_h_
#define _rheaAppEnumAndDefine_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"
#include "../SocketBridge/SocketBridgeEnumAndDefine.h"
#include "../SocketBridge/SocketBridgeFileTEnumAndDefine.h"

namespace rhea
{
	namespace app
	{
		extern rhea::NullLogger nullLogger;

		enum eDecodedMsgType
		{
			eDecodedMsgType_event		= 0x01,
			eDecodedMsgType_fileTransf	= 0x02,
			eDecodedMsgType_unknown		= 0xff
		};
		
		enum eFileTransferStatus
		{
			eFileTransferStatus_finished_KO	= 0,
			eFileTransferStatus_finished_OK	= 1,
			eFileTransferStatus_inProgress	= 2
		};


		struct sDecodedEventMsg
		{
			socketbridge::eEventType	eventType;
			u8							seqNumber;
			u16							payloadLen;
			const u8					*payload;
		};


		struct sDecodedFileTransfMsg
		{
			u16									payloadLen;
			const u8							*payload;
		};

		struct sDecodedMsg
		{
			eDecodedMsgType	what;
			union
			{
				sDecodedEventMsg		asEvent;
				sDecodedFileTransfMsg	asFileTransf;
			} data;
		};



	} // namespace app

} // namespace rhea


#endif // _rheaAppEnumAndDefine_h_

