#ifndef _CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo_h_
#define _CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo
     *
     * Si richiede l'inizio del test "assorbimento gruppo"
	 *	Dopo 1 sec circa dalla richiesta, è necessario "pollare" costantemente la CPU con il comando 0x20
     *	per conoscere lo stato della operazione
        Input:
            command: startTestAssGrp
            params:  none

		Output:
			OK
     */


    class CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "startTestAssGrp"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo_h_
