#include "CmdHandler_eventReqClientList.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../IdentifiedClientList.h"
#include "../../rheaCommonLib/rheaNetBufferView.h"
#include "../../rheaCommonLib//rheaStaticBuffer.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqClientList::handleRequestFromSocketBridge (socketbridge::Server *server, HSokServerClient &hClient)
{
	//NB: se modifichi questa, modifica anche rhea::app::ClientList::decodeAnswer()


	//il client vuole un elenco di client connessi
	const IdentifiedClientList *list = server->getIdentifieidClientList();

	//alloco un buffer grosso abbastanza per contenere tutte le info
	rhea::Allocator *allocator = rhea::getScrapAllocator();
	


	//il totale del payload non voglio che superi 2kb. Calcolo quindi quandi sIdentifiedClientInfo posso inviare al max
	const u16 MAX_SIZE_OF_PAYLOAD = 2048;
	const u16 SIZE_OF_INFO = 30;
	const u16 MAX_NUM_OF_INFO_TO_SEND = (MAX_SIZE_OF_PAYLOAD - 12) / SIZE_OF_INFO;

	u16 nClient = (u16)list->getCount();
	u16 nClientInfoSent = nClient;
	if (nClientInfoSent > MAX_NUM_OF_INFO_TO_SEND)
		nClientInfoSent = MAX_NUM_OF_INFO_TO_SEND;

	u32	bufferSize =
		sizeof(u16) //num client connessi
		+ sizeof(u16) //num client le cui informazioni sono state inserite in questo msg
		+ sizeof(u64) //data e ora di avvio dell'applicazione
		+ sizeof(sIdentifiedClientInfo) * nClientInfoSent; //uno per ogni client
	u8 *buffer = (u8*)RHEAALLOC(allocator, bufferSize);

	//NetBufferView per poter inserire i dati in maniera "indian indipendent"
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, bufferSize, rhea::eBigEndian);

	//serializzo i dati nel buffer
	nbw.writeU16(nClient);
	nbw.writeU16(nClientInfoSent);
	nbw.writeU64(rhea::getDateTimeAppStarted().getInternalRappresentation());
	
	for (u32 i = 0; i < nClientInfoSent; i++)
	{
		const sIdentifiedClientInfo *info = list->getInfoByIndex(i);

		nbw.writeU16(info->handle.index);
		nbw.writeU32(info->currentWebSocketHandleAsU32);

		nbw.writeU32(info->idCode.data.asU32);
		nbw.writeU8(info->clientVer.apiVersion);
		nbw.writeU8(info->clientVer.appType);
		nbw.writeU8(info->clientVer.unused2);
		nbw.writeU8(info->clientVer.unused3);

		nbw.writeU64(info->timeCreatedMSec);
		nbw.writeU64(info->lastTimeRcvMSec);
	}

	//invio
	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
