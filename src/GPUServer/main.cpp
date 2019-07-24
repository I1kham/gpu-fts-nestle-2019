#include "../rheaGUIBridge/GUIBridge.h"

struct sGPUThreadInitParam
{
    HThreadMsgR hRead;
    HThreadMsgW hWriteToServer;
};


u8 bSigTermKilled = 0;

/**************************************************************************
 * Risponde alle richieste di guiBrdige simulando la presenza di una GPU
 */
i16 gpuThreadFn (void *userParam)
{
    sGPUThreadInitParam *init = (sGPUThreadInitParam*)userParam;
    HThreadMsgR chReadFromServer = init->hRead;
    HThreadMsgW chWriteToServer = init->hWriteToServer;


    rhea::thread::sMsg  msg;
    OSEvent             hEventMsgAvail;
    rhea::thread::getMsgQEvent (chReadFromServer, &hEventMsgAvail);

    u64 timeToSendEndOfCurrentSelectionMSec = u64MAX;
    while (1)
    {
        OSEvent_wait(hEventMsgAvail, 5000);

        u64 timeNowMSec = OS_getTimeNowMSec();

        //se avevo una selezione in "preparazione", dopo un po' invio il messaggio di "selezione terminata"
        if (timeNowMSec >= timeToSendEndOfCurrentSelectionMSec)
        {
            timeToSendEndOfCurrentSelectionMSec = u64MAX;
            //invio eSelectionReqStatus_finished
            guibridge::CmdHandler_selStatus_buildAResponseAndPushItToServer (chWriteToServer, 4);
        }


        while (rhea::thread::popMsg(chReadFromServer, &msg))
        {
            if (msg.what == GUIBRIDGE_SERVER_DYING)
            {
                rhea::thread::deleteMsg(msg);
                return 1;
            }

            if ((msg.what & 0xFF00) == 0x0100)
            {
                //è un msg da parte del server che prevede un handlerID da utilizzare per la risposta
                u16 handlerID = (u16)(msg.paramU32 & 0x0000FFFF);

                switch (msg.what)
                {
                default:
                    //comando non suportato...
                    break;

                case GUIBRIDGE_REQ_SELAVAILABILITY:
                    {
                        //rispondo con una lista selezioin tutte abilitate
                        const u8 NUM_SEL = 48;
                        const u8 NUM_BYTES = 1 + NUM_SEL/8;
                        unsigned int availList[NUM_BYTES];
                        memset (availList, 0xff, NUM_BYTES);
                        guibridge::CmdHandler_selAvailability_buildAResponseAndPushItToServer (chWriteToServer, handlerID, NUM_SEL, availList);
                    }
                    break;

                case GUIBRIDGE_REQ_SELPRICES:
                    {
                        //rispondo con una lista prezzi tutta a zero
                        const u8 NUM_SEL = 48;
                        unsigned int priceList[NUM_SEL];
                        memset (priceList, 0, sizeof(priceList));
                        guibridge::CmdHandler_selPrices_buildAResponseAndPushItToServer (chWriteToServer, handlerID, NUM_SEL, priceList);

                    }
                    break;

                case GUIBRIDGE_REQ_STARTSELECTION:
                    //invio eSelectionReqStatus_preparing
                    guibridge::CmdHandler_selStatus_buildAResponseAndPushItToServer (chWriteToServer, 2);

                    timeToSendEndOfCurrentSelectionMSec = timeNowMSec + 4000;
                    break;

                case GUIBRIDGE_REQ_STOPSELECTION:
                    //invio eSelectionReqStatus_finished
                    guibridge::CmdHandler_selStatus_buildAResponseAndPushItToServer (chWriteToServer, 4);
                    timeToSendEndOfCurrentSelectionMSec = u64MAX;
                    break;

                case GUIBRIDGE_REQ_CREDIT:
                    break;

                } //switch (msg.what)
            }

            rhea::thread::deleteMsg(msg);
        }
    }
}

//*****************************************************
int main()
{
    rhea::init(NULL);

    //inizializza il webserver e recupera un handle sul quale è possibile restare in ascolto per vedere
    //quando il server invia delle richieste (tipo: WEBSOCKET_COMMAND_START_SELECTION)
    HThreadMsgR hQMessageFromWebserver;
    HThreadMsgW hQMessageToWebserver;
    rhea::HThread hServer;
    if (!guibridge::startServer(&hServer, &hQMessageFromWebserver, &hQMessageToWebserver))
    {
        printf ("server > failed to start...");
        return -1;
    }


    //creo un thread che si occupa di fare le veci della GPU, rispondendo alle eventuali richieste che arrivano da guibridge::server
    rhea::HThread hGPU;
    {
        sGPUThreadInitParam    init;
        init.hRead = hQMessageFromWebserver;
        init.hWriteToServer = hQMessageToWebserver;

        rhea::thread::create (&hGPU, gpuThreadFn, &init);
    }


    //attendo che il thread del server muoia
    rhea::thread::waitEnd(hServer);

    printf ("server killed\n");
    rhea::deinit();

    printf ("fin\n");
    return 0;
}

