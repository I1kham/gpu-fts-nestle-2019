#ifndef _CmdHandler_h_
#define _CmdHandler_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"
#include "SocketBridgeEnumAndDefine.h"


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

		virtual bool		needToPassDownToCPUBridge() const = 0;
        virtual void		onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
		
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
