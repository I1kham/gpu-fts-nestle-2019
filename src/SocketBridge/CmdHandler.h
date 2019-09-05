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
                        CmdHandler (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec);
        virtual         ~CmdHandler()                                              {}

        virtual void    handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge) = 0;

        u16             getHandlerID() const																					{ return handlerID; }
        bool            isTimeToDie(u64 timeNowMSec) const																		{ return (timeNowMSec > timeToDieMSec); }

    protected:
        HSokServerClient   hClient;

    private:
        u16             handlerID;
        u64             timeToDieMSec;
    };
} // namespace socketbridge

#endif // _CmdHandler_h_
