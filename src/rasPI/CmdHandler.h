#ifndef _CmdHandler_h_
#define _CmdHandler_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "rasPIEnumAndDefine.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler
     *
     */
    class CmdHandler
    {
    public:
							CmdHandler (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec);
        virtual				~CmdHandler()                                              {}

		virtual bool		needForwardToGPU() const = 0;
        virtual void		onGPUNotification (rasPI::socketListener::Core *core, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
		
        u16					getHandlerID() const																					{ return handlerID; }
        bool				isTimeToDie(u64 timeNowMSec) const																		{ return (timeNowMSec > timeToDieMSec); }
		HSokBridgeClient	getIdentifiedClientHandle() const																		{ return identifiedClientHandle; }

    protected:
		HSokBridgeClient   identifiedClientHandle;

    private:
        u16					handlerID;
        u64					timeToDieMSec;
    };
} // namespace socketbridge

#endif // _CmdHandler_h_
