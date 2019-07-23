#include "IdentifiedClientList.h"

using namespace guibridge;


//*********************************************************
void IdentifiedClientList::setup (rhea::Allocator *allocator)
{
    list.setup (allocator, 32);
}


//*********************************************************
void IdentifiedClientList::unsetup()
{
    list.unsetup();
}


//*********************************************************
const IdentifiedClientList::sInfo* IdentifiedClientList::isKwnownClient (const HWebsokClient &h) const
{
    u32 i = priv_findClientByHWebSocket(h);
    if (u32MAX == i)
        return NULL;

    return &list(i);
}


//*********************************************************
u32 IdentifiedClientList::priv_findClientByHWebSocket (const HWebsokClient &h) const
{
    const u32 hAsU32 = h.asU32();
    u32 n = list.getNElem();
    for (u32 i=0; i<n; i++)
    {
        if (list(i).currentWebSocketHandleAsU32 == hAsU32)
            return i;
    }

    return u32MAX;
}

//*********************************************************
bool IdentifiedClientList::registerClient (const HWebsokClient &h, u32 identificationCode, u8  apiVersion, bool *out_bWasNew)
{
    *out_bWasNew = false;

    const u32 hAsU32 = h.asU32();
    u32 n = list.getNElem();
    for (u32 i=0; i<n; i++)
    {
        //se ho già un client con lo stesso identificationCode in lista...
        if (list(i).identificationCode == identificationCode)
        {
            //..e la stessa apiVersion, allora tutto ok, devo solo memorizzare il nuovo HWebsokClient
            if (list(i).apiVersion == apiVersion)
            {
                list[i].currentWebSocketHandleAsU32 = hAsU32;
                list[i].lastTimeRegisteredMSec = OS_getTimeNowMSec();
                return true;
            }

            //altrimenti è un errore, la coppia [identificationCode,apiVersion] deve essere univoca
            unregisterClient(h);
            return false;
        }

        //se ho già questo handle in lista, allora deve per forza avere lo srtesso id a apiversion
        if (list(i).currentWebSocketHandleAsU32 == hAsU32)
        {
            if (list(i).identificationCode == identificationCode && list(i).apiVersion == apiVersion)
            {
                list[i].lastTimeRegisteredMSec = OS_getTimeNowMSec();
                return true;
            }

            unregisterClient(h);
            return false;
        }
    }

    //ok, era un client nuovo, lo addo alla lista
    list[n].currentWebSocketHandleAsU32 = hAsU32;
    list[n].apiVersion = apiVersion;
    list[n].identificationCode = identificationCode;
    list[n].firstTimeRegisteredMSec = list[n].lastTimeRegisteredMSec = OS_getTimeNowMSec();
    *out_bWasNew = true;
    return true;
}

//*********************************************************
void IdentifiedClientList::unregisterClient (const HWebsokClient &h)
{
    const u32 hAsU32 = h.asU32();
    u32 n = list.getNElem();
    for (u32 i=0; i<n; i++)
    {
        if (list(i).currentWebSocketHandleAsU32 == hAsU32)
        {
            printf ("server> IdentifiedClientList::unregisterClient()  [id:0x%08X] [apiv:0x%02X] [h:0x%02X]\n", list(i).identificationCode, list(i).apiVersion, list(i).currentWebSocketHandleAsU32);

            list.removeAndSwapWithLast(i);
            return;
        }
    }
}
