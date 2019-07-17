#ifndef _CmdHandler_h_
#define _CmdHandler_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "Websocket/WebsocketServer.h"
#include "GUIBridgeEnumAndDefine.h"


namespace guibridge
{
    /*********************************************************
     * CmdHandler
     *
     */
    class CmdHandler
    {
    public:
                        CmdHandler (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec);
        virtual         ~CmdHandler()                                              {}


        virtual void    handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU) = 0;

        u16             getHandlerID() const                                                { return handlerID; }
        bool            isTimeToDie(u64 timeNowMSec) const                                  { return (timeNowMSec > timeToDieMSec); }

    protected:
        HWebsokClient   hClient;

    private:
        u16             handlerID;
        u64             timeToDieMSec;
    };
} // namespace guibridge

#endif // _CmdHandler_h_
