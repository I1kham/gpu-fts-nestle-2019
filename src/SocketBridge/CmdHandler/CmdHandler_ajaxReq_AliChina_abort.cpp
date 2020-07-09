#include "CmdHandler_ajaxReq_AliChina_abort.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_AliChina_abort::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	server->module_alipayChina_abort();

	u8 data[4];
	data[0] = 'O';
	data[1] = 'K';
	data[2] = 0x00;
	server->sendAjaxAnwer (hClient, ajaxRequestID, data, 3);
}