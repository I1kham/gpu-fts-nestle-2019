#ifndef _CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest_h_
#define _CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest
     *
     *
        Input:
            command: startGrnSpeedTest
            params:
				m: 1,2,3,4
				d: durata in sec (intero)

		Output
			OK
			KO

		Alla ricezione del comando, CPUBridge risponde OK ed entra nello stato [eVMCState::GRINDER_SPEED_TEST] (106) e inzia a macinare per il tempo [d] usando la macina [m].
		Durante l'intera macinata, raccoglie informazioni sul valore riportato dal sensore della macina (che esiste solo in determinate macine).
		Alla fine dell'operazione, è possibile usare il comando [CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue]  per recuperare il valor medio del sensore calcolato durante l'intera macinata

		In caso di fallimento, CPU risponde KO e lo stato non cambia
     */


    class CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "startGrnSpeedTest"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqAttivazioneMotore_h_
