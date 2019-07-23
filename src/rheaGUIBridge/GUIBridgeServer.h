#ifndef _GUIBridgeServer_h_
#define _GUIBridgeServer_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "Websocket/WebsocketServer.h"
#include "GUIBridgeEnumAndDefine.h"
#include "CommandHandlerList.h"
#include "IdentifiedClientList.h"

namespace guibridge
{
    class Server
    {
    public:
                                Server();
                                ~Server()               { close(); }

        bool                    open (u16 serverPort, const HThreadMsgR hQMessageIN, const HThreadMsgW hQMessageOUT);
        void                    close();
        void                    run();

    private:
        static const u16        RESERVED_HANDLE_RANGE = 1024;



    private:
        WebsocketServer         *server;
        rhea::Allocator         *localAllocator;
        rhea::LinearBuffer      buffer;
        IdentifiedClientList    identifiedClientList;
        CommandHandlerList      cmdHandlerList;
        HThreadMsgR             hQMessageIN;
        HThreadMsgW             hQMessageOUT;
        OSEvent                 evFromThreadMsgQ;
        u64                     nextTimePurgeCmdHandlerList;
        u16                     _nextHandlerID;
        bool                    bQuit;

    private:
        u16                     priv_getANewHandlerID ();
        void                    priv_onClientHasDataAvail (u8 iEvent);
        void                    priv_onGPUEvent (rhea::thread::sMsg &msg);
        void                    priv_onConsoleEvent (rhea::thread::sMsg &msg);
    };

} // namespace guibridge

#endif // _GUIBridgeServer_h_
