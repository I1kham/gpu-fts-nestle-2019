#ifndef _CommandHandlerList_h_
#define COMMANDHANDLERLIST_H
#include "CmdHandler.h"
#include "../rheaCommonLib/rheaMemory.h"


namespace guibridge
{
    /***************************************************
     * CommandHandlerList
     *
     * lista di CmdHandler
     */
    class CommandHandlerList
    {
    public:
                                CommandHandlerList()                                        { }
                                ~CommandHandlerList()                                       { unsetup(); }

        void                    setup (rhea::Allocator *allocator)                          { list.setup (allocator, 64); }
        void                    unsetup()                                                   { list.unsetup(); }

        void                    add (CmdHandler *h)                                { list.append(h); }

        CmdHandler*             findByID (u16 handlerID) const
                                {
                                    u32 n = list.getNElem();
                                    for (u32 i=0; i<n; i++)
                                    {
                                        if (list.getElem(i)->getHandlerID() == handlerID)
                                            return list.getElem(i);
                                    }
                                    return NULL;
                                }

        void                    removeAndDeleteByID (rhea::Allocator *allocator, u16 handlerID)
                                {
                                    u32 n = list.getNElem();
                                    for (u32 i=0; i<n; i++)
                                    {
                                        CmdHandler *h = list.getElem(i);
                                        if (h->getHandlerID() == handlerID)
                                        {
                                            RHEADELETE(allocator, h);
                                            list.removeAndSwapWithLast(i);
                                            return;
                                        }
                                    }
                                }

        void                    purge (rhea::Allocator *allocator, u64 timeNowMSec)
                                {
                                    u32 n = list.getNElem();
                                    for (u32 i=0; i<n; i++)
                                    {
                                        CmdHandler *h = list(i);
                                        if (h->isTimeToDie(timeNowMSec))
                                        {
                                            RHEADELETE(allocator, h);
                                            list.removeAndSwapWithLast(i);
                                            n--;
                                            i--;
                                        }
                                    }
                                }

    private:
        rhea::FastArray<CmdHandler*> list;

    };
} //namespace guibridge
#endif // _CommandHandlerList_h_
