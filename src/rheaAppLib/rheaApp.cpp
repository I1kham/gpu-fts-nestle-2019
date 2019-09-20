#include "rheaApp.h"
#include "rheaAppEnumAndDefine.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"
#include "../rheaCommonLib//rheaStaticBuffer.h"
#include "../SocketBridge/SocketBridgeFileTEnumAndDefine.h"

namespace rhea
{
	namespace app
	{
		NullLogger nullLogger;
	}
}

using namespace rhea;

/*****************************************************************
 * In input, [in_out_bufferLength] deve contenere la dimensione di[out_buffer]
 * Ritorna:
 * false in caso di problemi.In questo caso, [in_out_bufferLength] contiene la dimensione minima necessaria per[out_buffer]
 *
 * Se ritorna true, allora[out_buffer] è stato fillato con i dati necessari per inviare l'evento a SocketBridge.
 * [in_out_bufferLength] contiene il num di bytes messi in[out_buffer];
 */
bool priv_prepareCommandBuffer (char commandChar, u8 requestID, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength)
{
	const u16 bytesNeeded =

		1 +					//# 
		1 +					//char del comando
		1 +					//requestID 
		2 +					//lenOfOptionalData espressa come byte alto, byte basso
		lenOfOptionalData +	//[optionalData]
		2;					//cheksum espressa come byte alto, byte basso

	if (*in_out_bufferLength < bytesNeeded)
	{
		DBGBREAK;
		*in_out_bufferLength = bytesNeeded;
		return false;
	}

	//fillo il buffer
	u16 ct = 0;
	out_buffer[ct++] = 35; //'#'
	out_buffer[ct++] = (u8)commandChar;
	out_buffer[ct++] = requestID;
	out_buffer[ct++] = ((lenOfOptionalData & 0xFF00) >> 8);
	out_buffer[ct++] = (lenOfOptionalData & 0x00FF);

	if (lenOfOptionalData)
	{
		memcpy(&out_buffer[ct], optionalData, lenOfOptionalData);
		ct += lenOfOptionalData;
	}

	//checksum
	const u16 ck = rhea::utils::simpleChecksum16_calc(out_buffer, ct);
	out_buffer[ct++] = ((ck & 0xFF00) >> 8);
	out_buffer[ct++] = (ck & 0x00FF);

	*in_out_bufferLength = ct;
	return true;
}



//*****************************************************************
void priv_event_sendToSocketBridge (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const void *optionalData, u8 sizeoOfOptionalData)
{
	u8	sendBuffer[32]; //32 bytes dovrebbero essere più che suff per ogni tipo di richiesta che le app inviano a socketbridge
	u16 sizeOSendfBuffer = sizeof(sendBuffer);

	if (priv_prepareCommandBuffer(socketbridge::eOpcode_event_E, 0xff, optionalData, sizeoOfOptionalData, sendBuffer, &sizeOSendfBuffer))
	{
		proto->write(ch, sendBuffer, sizeOSendfBuffer, 1000);
		return;
	}

	DBGBREAK;
}

//*****************************************************************
u16 priv_event_decode (const void *bufferIN, u16 nBytesInBuffer, app::sDecodedEventMsg *out)
{
	//come minimo devono esserci 8 byte
	if (nBytesInBuffer < 8)
		return 0;

	const u8 *buffer = static_cast<const u8*>(bufferIN);

	if (buffer[0] == '#' && buffer[1] == 'e' && buffer[2] == 'V' && buffer[3] == 'n')
	{
		out->eventType = (socketbridge::eEventType)buffer[4];
		out->seqNumber = buffer[5];
		out->payloadLen = buffer[6];
		out->payloadLen <<= 8;
		out->payloadLen |= buffer[7];

		if (out->payloadLen)
			out->payload = &buffer[8];
		else
			out->payload = NULL;
		return 8 + out->payloadLen;
	}
	
	return 0;
}


//*****************************************************************
u16 priv_RawFileTrans_decodeMsg(const void *bufferIN, u16 nBytesInBuffer, app::sDecodedFileTransfMsg *out)
{
	//come minimo devono esserci 8 byte
	if (nBytesInBuffer < 8)
		return 0;

	const u8 *buffer = static_cast<const u8*>(bufferIN);

	if (buffer[0] == '#' && buffer[1] == 'f' && buffer[2] == 'T' && buffer[3] == 'r')
	{
		out->payloadLen = buffer[4];
		out->payloadLen <<= 8;
		out->payloadLen |= buffer[5];

		u32 ct = 6;
		if (out->payloadLen)
		{
			out->payload = &buffer[6];
			ct += out->payloadLen;
		}
		else
			out->payload = NULL;
		
		const u16 ck = (((u16)buffer[ct]) << 8) | buffer[ct + 1];
		if (ck != rhea::utils::simpleChecksum16_calc(buffer, ct))
			return 0;

		return ct+2;
	}

	return 0;
}



//*****************************************************************
u16 app::decodeSokBridgeMessage(const void *bufferIN, u16 nBytesInBuffer, app::sDecodedMsg *out)
{
	u16 ret = priv_event_decode(bufferIN, nBytesInBuffer, &out->data.asEvent);
	if (ret)
	{
		out->what = eDecodedMsgType_event;
		return ret;
	}

	ret = priv_RawFileTrans_decodeMsg(bufferIN, nBytesInBuffer, &out->data.asFileTransf);
	if (ret)
	{
		out->what = eDecodedMsgType_fileTransf;
		return ret;
	}

	out->what = eDecodedMsgType_unknown;
	return 0;
}




//*****************************************************************
void app::send_requestIDCodeAfterConnection(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const socketbridge::SokBridgeClientVer &version)
{
	u8	buffer[32];
	u16 sizeOfBuffer = sizeof(buffer);

	u8 optionalData[4];
	optionalData[0] = (u8)version.apiVersion;
	optionalData[1] = (u8)version.appType;
	optionalData[2] = (u8)version.unused2;
	optionalData[3] = (u8)version.unused3;

	if (priv_prepareCommandBuffer(socketbridge::eOpcode_request_idCode, 0xff, optionalData, sizeof(optionalData), buffer, &sizeOfBuffer))
	{
		proto->write(ch, buffer, sizeOfBuffer, 1000);
		return;
	}

	DBGBREAK;
}

//*****************************************************************
void app::send_identifyAfterConnection (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const socketbridge::SokBridgeClientVer &version, socketbridge::SokBridgeIDCode &idCode)
{
	u8	buffer[32];
	u16 sizeOfBuffer = sizeof(buffer);

	u8 optionalData[8];
	optionalData[0] = (u8)version.apiVersion;
	optionalData[1] = (u8)version.appType;
	optionalData[2] = (u8)version.unused2;
	optionalData[3] = (u8)version.unused3;
	optionalData[4] = (u8)idCode.data.buffer[0];
	optionalData[5] = (u8)idCode.data.buffer[1];
	optionalData[6] = (u8)idCode.data.buffer[2];
	optionalData[7] = (u8)idCode.data.buffer[3];

	if (priv_prepareCommandBuffer(socketbridge::eOpcode_identify_W, 0xff, optionalData, sizeof(optionalData), buffer, &sizeOfBuffer))
	{
		proto->write(ch, buffer, sizeOfBuffer, 1000);
		return;
	}

	DBGBREAK;
}



//*****************************************************************
bool app::handleInitialRegistrationToSocketBridge (rhea::ISimpleLogger *logIN, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, const socketbridge::SokBridgeClientVer &version, socketbridge::SokBridgeIDCode *out_idCode, u32 *out_SMUversion)
{
	rhea::ISimpleLogger *log = logIN;
	if (NULL == log)
		log = &nullLogger;


	log->log("handleInitialRegistrationToSocketBridge\n");
	log->incIndent();

	log->log("requesting new IDCode...\n");
	log->incIndent();
	{
		app::send_requestIDCodeAfterConnection(ch, proto, version);
		u16 nRead = proto->read(ch, 2000, bufferR);
		if (nRead >= rhea::protocol::RES_ERROR)
		{
			log->log("error reading from protocol. Err = %d\n", nRead);
			log->decIndent();
			log->decIndent();
			return false;
		}

		app::sDecodedMsg decoded;
		u16 nUsed = rhea::app::decodeSokBridgeMessage(bufferR._getPointer(0), nRead, &decoded);
		if (nUsed > 0)
		{
			if (decoded.what != eDecodedMsgType_event)
			{
				log->log("invalid message rcv. Was expecting an event\n");
				log->decIndent();
				log->decIndent();
				return false;
			}

			app::sDecodedEventMsg *decEvent = &decoded.data.asEvent;
			if (decEvent->payloadLen != 6 || decEvent->eventType != socketbridge::eEventType_answer_to_idCodeRequest)
			{
				log->log("invalid message rcv. payload len is [%d], expect was 6. Eventtype is[%d], expected was %d\n", decEvent->payloadLen, decEvent->eventType, socketbridge::eEventType_answer_to_idCodeRequest);
				log->decIndent();
				log->decIndent();
				return false;
			}
			*out_SMUversion = ((u32)decEvent->payload[0]) << 8; //cpuBridge Version
			*out_SMUversion |= decEvent->payload[1]; //socketBridge Version

			out_idCode->data.buffer[0] = decEvent->payload[2];
			out_idCode->data.buffer[1] = decEvent->payload[3];
			out_idCode->data.buffer[2] = decEvent->payload[4];
			out_idCode->data.buffer[3] = decEvent->payload[5];
			log->log("rcv idCode [0x%08X]\n", out_idCode->data.asU32);
		}
	}
	log->decIndent();


	log->log("sending idCode...\n");
	log->incIndent();
	{
		app::send_identifyAfterConnection (ch, proto, version, *out_idCode);
		log->log("done\n");
	}
	log->decIndent();

	log->decIndent();
	return true;
}






/*****************************************************************
 * CurrentSelectionRunningStatus
 */
void app::CurrentSelectionRunningStatus::decodeAnswer (const sDecodedEventMsg &msg, cpubridge::eRunningSelStatus *out)
{
	assert(msg.eventType == socketbridge::eEventType_selectionRequestStatus);
	assert(msg.payloadLen >= 1);
	*out = (cpubridge::eRunningSelStatus)msg.payload[0];
}



/****************************************************************
 * CurrentCPUMessage
 */
void app::CurrentCPUMessage::ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto)
{
	u8 optionalData[4];
	optionalData[0] = (u8)socketbridge::eEventType_cpuMessage;
	priv_event_sendToSocketBridge(ch, proto, optionalData, 1);
}

void app::CurrentCPUMessage::decodeAnswer(const sDecodedEventMsg &msg, u8 *out_msgImportanceLevel, u16 *out_msgLenInBytes, u8 *out_messageInUTF8, u32 sizeOfOutMessage)
{
	assert(msg.eventType == socketbridge::eEventType_cpuMessage);
	assert(msg.payloadLen >= 2);
	*out_msgImportanceLevel = msg.payload[0];
	*out_msgLenInBytes = (((u16)msg.payload[1]) << 8) | msg.payload[2];

	memset(out_messageInUTF8, 0, sizeOfOutMessage);
	if (*out_msgLenInBytes > 0)
	{
		assert(msg.payloadLen == *out_msgLenInBytes + 3);
		const u8 *m = &msg.payload[3];

		u32 ct = 0;
		u32 i = 0;
		while (i < *out_msgLenInBytes)
		{
			u8 byte1 = m[i++];
			u8 byte2 = m[i++];

			out_messageInUTF8[ct++] = byte1;
			if (byte2 != 0x00)
				out_messageInUTF8[ct++] = byte2;
		}
	}
}



/****************************************************************
 * CurrentCPUStatus
 */
void app::CurrentCPUStatus::ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto)
{
	u8 optionalData[4];
	optionalData[0] = (u8)socketbridge::eEventType_cpuStatus;
	priv_event_sendToSocketBridge(ch, proto, optionalData, 1);
}

void app::CurrentCPUStatus::decodeAnswer (const sDecodedEventMsg &msg, cpubridge::eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType)
{
	assert(msg.eventType == socketbridge::eEventType_cpuStatus);
	assert(msg.payloadLen >= 3);
	*out_VMCstate = (cpubridge::eVMCState)msg.payload[0];
	*out_VMCerrorCode = msg.payload[1];
	*out_VMCerrorType = msg.payload[2];
}



/****************************************************************
 * ClientList
 */
void app::ClientList::ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto)
{
	u8 optionalData[4];
	optionalData[0] = (u8)socketbridge::eEventType_reqClientList;
	priv_event_sendToSocketBridge(ch, proto, optionalData, 1);
}

void app::ClientList::decodeAnswer (const sDecodedEventMsg &msg, rhea::Allocator *allocator,u16 *out_nClientConnected, u16 *out_nClientInfo, rhea::DateTime *out_dtCPUBridgeStarted, socketbridge::sIdentifiedClientInfo **out_list)
{
	assert(msg.eventType == socketbridge::eEventType_reqClientList);

	//static buffer che "punta" al buffer di msg
	rhea::StaticBufferReadOnly sb;
	sb.setup(msg.payload, msg.payloadLen);


	//NetBufferView per poter leggere i dati in maniera "indian indipendent"
	NetStaticBufferViewR nbr;
	nbr.setup(msg.payload, msg.payloadLen, rhea::eBigEndian);

	u16 nClient, nClientInfo;
	u64 u;
	nbr.readU16(nClient);
	nbr.readU16(nClientInfo);
	nbr.readU64(u);
	out_dtCPUBridgeStarted->setFromInternalRappresentation(u);

	if (nClient == 0)
	{
		*out_nClientConnected = *out_nClientInfo = 0;
		*out_list = NULL;
		return;
	}
	
	*out_nClientConnected = nClient;
	*out_nClientInfo = nClientInfo;
	*out_list = (socketbridge::sIdentifiedClientInfo*)RHEAALLOC(allocator, sizeof(socketbridge::sIdentifiedClientInfo) * nClientInfo);
	for (u32 i = 0; i < nClientInfo; i++)
	{
		socketbridge::sIdentifiedClientInfo *info = &(*out_list)[i];

		nbr.readU16(info->handle.index);
		nbr.readU32(info->currentWebSocketHandleAsU32);

		nbr.readU32(info->idCode.data.asU32);
		nbr.readU8(info->clientVer.apiVersion);
		nbr.readU8(info->clientVer.appType);
		nbr.readU8(info->clientVer.unused2);
		nbr.readU8(info->clientVer.unused3);

		nbr.readU64(info->timeCreatedMSec);
		nbr.readU64(info->lastTimeRcvMSec);
	}

}


/*****************************************************************
 * CurrentSelectionAvailability
 */
void app::CurrentSelectionAvailability::ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto)
{
	u8 optionalData[4];
	optionalData[0] = (u8)socketbridge::eEventType_selectionAvailabilityUpdated;
	priv_event_sendToSocketBridge(ch, proto, optionalData, 1);
}

void app::CurrentSelectionAvailability::decodeAnswer(const sDecodedEventMsg &msg, u8 *out_numSel, u8 *out_selectionAvailability, u32 sizeOfSelecionAvailability)
{
	assert(msg.eventType == socketbridge::eEventType_selectionAvailabilityUpdated);
	assert(msg.payloadLen > 1);
	
	u8 n = *out_numSel = msg.payload[0];

	//serve 1 byte per ogni selezione
	if (n > sizeOfSelecionAvailability)
		n = sizeOfSelecionAvailability;
	for (u8 i = 0; i < n; i++)
		out_selectionAvailability[i] = msg.payload[1 + i];
}






/*****************************************************************
 * RawFileTrans
 */
 //*****************************************************************
void app::RawFileTrans::sendToSocketBridge (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u8 *sendBuffer, u32 sizeOfSendBufferIN, const void *optionalData, u16 sizeoOfOptionalData)
{
	u16 sizeOSendfBuffer = sizeOfSendBufferIN;

	if (priv_prepareCommandBuffer(socketbridge::eOpcode_fileTransfer, 0xfe, optionalData, sizeoOfOptionalData, sendBuffer, &sizeOSendfBuffer))
	{
		proto->write(ch, sendBuffer, sizeOSendfBuffer, 1000);
		return;
	}

	DBGBREAK;
}

