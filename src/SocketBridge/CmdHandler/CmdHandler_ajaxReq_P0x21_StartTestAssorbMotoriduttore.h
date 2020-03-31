#ifndef _CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore_h_
#define _CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore
     *
     * Si richiede l'inizio del test "assorbimento motoriduttore"
	 *	Dopo 1 sec circa dalla richiesta, è necessario "pollare" costantemente la CPU con il comando 0x22
     *	per conoscere lo stato della operazione
        Input:
            command: startTestAssMotorid
            params:  none

		Output:
			OK
     */


    class CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "startTestAssMotorid"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore_h_
