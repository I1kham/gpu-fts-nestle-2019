#ifndef _GUIBridgeEnumAndDefine_h_
#define _GUIBridgeEnumAndDefine_h_
#include "../rheaCommonLib/rheaThread.h"

/* elenco dei comandi che il server invia lungo il suo canale di output*/
#define GUIBRIDGE_SERVER_STARTED                    0x0001

/*  Richieste dal server di GUI alla GPU
    Per convenzioni, il MSB deve essere 0x01 */
#define GUIBRIDGE_REQ_SELAVAILABILITY               0x0100
#define GUIBRIDGE_REQ_SELPRICES                     0x0101
#define GUIBRIDGE_REQ_STARTSELECTION                0x0102
#define GUIBRIDGE_REQ_STOPSELECTION                 0x0103
#define GUIBRIDGE_SELECTION_REQ_STATUS              0x0104
#define GUIBRIDGE_REQ_CPU_MESSAGE                   0x0105
#define GUIBRIDGE_REQ_CREDIT                        0x0106

/* richieste dal server alla GPU */
#define GUIBRIDGE_GPU_EVENT                        0x0200


namespace guibridge
{
    struct sServerInitParam
    {
        HThreadMsgR hQMessageIN;      //il thread del server riceve messaggi dal resto del mondo su questo canale
        HThreadMsgW hQMessageOUT;     //il thread del server invia messaggi al resto del mondo su questo canale
    };

    enum eEventType
    {
        eEventType_selectionAvailabilityUpdated  = 'a',
        eEventType_selectionPricesUpdated  = 'b',
        eEventType_creditUpdated = 'c',
        eEventType_cpuMessage = 'd',

        eEventType_selectionRequestStatus = 'e',
        eEventType_startSelection = 'f',
        eEventType_stopSelection = 'g',

        eEventType_unknown = 0xff
    };
} // namespace guibridge


#endif // _GUIBridgeEnumAndDefine_h_

