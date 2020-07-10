#include "CmdHandler_ajaxReq_AliChina_isOnline.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_AliChina_isOnline::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	u8 data[4];
	if (server->module_alipayChina_isOnline())
	{
		data[0] = 'O';
		data[1] = 'K';
	}
	else
	{
		data[0] = 'K';
		data[1] = 'O';
	}
	data[2] = 0x00;
	server->sendAjaxAnwer (hClient, ajaxRequestID, data, 3);
}