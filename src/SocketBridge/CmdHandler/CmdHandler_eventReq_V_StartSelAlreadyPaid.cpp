#include "CmdHandler_eventReq_V_StartSelAlreadyPaid.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaUtils.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_V_StartSelAlreadyPaid::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    const u8 selNum = payload[1];
    const cpubridge::eGPUPaymentType paymentType = (cpubridge::eGPUPaymentType)payload[2];
    const u16 price = rhea::utils::bufferReadU16 (&payload[3]);

	cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (from, selNum, price, paymentType);
}

//***********************************************************
void CmdHandler_eventReq_V_StartSelAlreadyPaid::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
