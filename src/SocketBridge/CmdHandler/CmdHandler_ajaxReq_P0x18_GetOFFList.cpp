#include "CmdHandler_ajaxReq_P0x18_GetOFFList.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaCommonLib/rheaString.h"


using namespace socketbridge;


struct sInput
{
	u8		startIndex;
};

//***********************************************************
bool ajaxReqGetOFFList_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "startIndex") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h < 0xff)
			input->startIndex = (u8)h;
		else
			input->startIndex = 0xff;
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReq_P0x18_GetOFFList::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqGetOFFList_jsonTrapFunction, &data))
	{
		if (data.startIndex != 0xff)
			cpubridge::ask_CPU_GET_OFF_REPORT(from, getHandlerID(), data.startIndex);
	}
}

//***********************************************************
void CmdHandler_ajaxReq_P0x18_GetOFFList::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 lastIndex = 0;
	u8 startIndex = 0;
	u8 numOffs = 0;
	cpubridge::sCPUOffSingleEvent offList[32];
	cpubridge::translateNotify_GET_OFF_REPORT(msgFromCPUBridge, &startIndex, &lastIndex, &numOffs, offList, sizeof(offList));

	if (numOffs > 0)
	{
		rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
		const u32 sizeOfBuffer = (40 * numOffs) + 128;
		u8 *buffer = (u8*)RHEAALLOC(allocator, sizeOfBuffer);

		u8 nextIndex = 0;
		if (lastIndex < 19)
			nextIndex = lastIndex + 1;
		sprintf_s((char*)buffer, sizeOfBuffer, "{\"nextIndex\":%d,", nextIndex);

		//"yyyy" : [2020, 2019]
		rhea::string::append ((char*)buffer, sizeOfBuffer, "\"yyyy\":[");
		rhea::string::append((char*)buffer, sizeOfBuffer, 2000 + offList[0].anno);
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",");
			rhea::string::append((char*)buffer, sizeOfBuffer, 2000 + offList[i].anno);
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");
				
		//,"mm" : ["01", "02"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"mm\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].mese, 2);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].mese, 2);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");
		
		//,"dd" : ["03", "04"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"dd\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].giorno, 2);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].giorno, 2);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");
		
		//,"hh" : ["03", "04"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"hh\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].ora, 2);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].ora, 2);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");
		
		//,"min" : ["03", "04"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"min\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].minuto, 2);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].minuto, 2);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");

		
		//,"codice" : ["8", "7"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"codice\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].codice);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].codice);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");


		//,"tipo" : ["A", " "]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"tipo\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, (char)offList[0].tipo);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, (char)offList[i].tipo);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");

		//,"stato" : ["0", "1"]
		rhea::string::append((char*)buffer, sizeOfBuffer, ",\"stato\":[\"");
		rhea::string::append((char*)buffer, sizeOfBuffer, offList[0].stato);
		rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		for (u8 i = 1; i < numOffs; i++)
		{
			rhea::string::append((char*)buffer, sizeOfBuffer, ",\"");
			rhea::string::append((char*)buffer, sizeOfBuffer, offList[i].stato);
			rhea::string::append((char*)buffer, sizeOfBuffer, "\"");
		}
		rhea::string::append((char*)buffer, sizeOfBuffer, "]");


		rhea::string::append((char*)buffer, sizeOfBuffer, "}");
				
		
		server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, (u16)strlen((const char*)buffer));
		RHEAFREE(allocator, buffer);
	}
}
