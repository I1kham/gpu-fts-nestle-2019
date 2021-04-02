#include "CmdHandler_ajaxReq_AliChina_getConnDetail.h"
#include "../SocketBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_AliChina_getConnDetail::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	char serverIP[16];
	u16 serverPort = 0;
	char machineID[64];
	char key[128];


	char answer[256];
	if (server->module_getConnectionDetail (serverIP, &serverPort, machineID, key))
	{
		u8 ip1, ip2, ip3, ip4;
		rhea::netaddr::ipstrTo4bytes(serverIP, &ip1, &ip2, &ip3, &ip4);
		sprintf_s (answer, sizeof(answer), "{\"ip1\":%d,\"ip2\":%d,\"ip3\":%d, \"ip4\":%d,\"port\":%d,\"mid\":\"%s\",\"key\":\"%s\"}",
			ip1, ip2, ip3, ip4, serverPort, machineID, key);
	}
	else
	{
		sprintf_s (answer, sizeof(answer), "{\"ip1\":0,\"ip2\":0,\"ip3\":0, \"ip4\":0,\"port\":0,\"mid\":\"???\",\"key\":\"???\"}");
	}
	
	server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)answer, (u16)strlen(answer));


}