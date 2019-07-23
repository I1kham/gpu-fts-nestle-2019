#ifndef _IdentifiedClientList_h_
#define _IdentifiedClientList_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/Websocket/WebsocketServer.h"

namespace guibridge
{
    /***************************************************
     * IdentifiedClientList
     *
     * lista di CmdHandler
     */
    class IdentifiedClientList
    {
    public:
        struct sInfo
        {
            u32     identificationCode;
            u32     currentWebSocketHandleAsU32;
            u64     firstTimeRegisteredMSec;
            u64     lastTimeRegisteredMSec;
            u8      apiVersion;
        };

    public:
                        IdentifiedClientList()                          { }
                        ~IdentifiedClientList()                         { unsetup(); }

        void            setup (rhea::Allocator *allocator);
        void            unsetup();

        const sInfo*    isKwnownClient (const HWebsokClient &h) const;
        bool            registerClient (const HWebsokClient &h, u32 identificationCode, u8  apiVersion, bool *out_bWasNew);
        void            unregisterClient (const HWebsokClient &h);

        u32             getNElem() const                                { return list.getNElem(); }
        const sInfo*    getElemByIndex (u32 i) const                    { return &list(i); }

    private:
        rhea::FastArray<sInfo>  list;

    private:
        u32             priv_findClientByHWebSocket (const HWebsokClient &h) const;
    };
} // namespace guibridge
#endif // _IdentifiedClientList_h_
