#include "../rheaGUIBridge/GUIBridge.h"

struct sConsoleInitParam
{
    HThreadMsgR hRead;
    HThreadMsgW hWriteToServer;
};


//**************************************************************************
i16 consoleThreadFn (void *userParam)
{
    sConsoleInitParam *init = (sConsoleInitParam*)userParam;
    //HThreadMsgR chRead = init->hRead;
    HThreadMsgW chWriteToServer = init->hWriteToServer;


    printf ("console> starting...\n");
    printf ("console> type quit to terminate\n");
    while (1)
    {
        char str[64];
        printf ("console>");

        char c;
        u16 ct=0;
        str[0] = 0x00;
        while ( (c = getchar()) )
        {
            if (c=='\n' || c==0x00)
            {
                str[ct] = 0;
                break;
            }

            str[ct++] = c;
            if (ct >= sizeof(str))
            {
                str[sizeof(str)-1] = 0;
                break;
            }

        }
        //scanf ("%s", str);

        if (strcasecmp(str,"quit") == 0)
        {
            printf ("console> quitting...\n");
            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_QUIT, 0);
            break;
        }
        else if (strcasecmp(str,"kill_server") == 0)
        {
            printf ("console> killing...\n");
            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_QUIT, 0);
        }
        else if (strcasecmp(str,"ffox") == 0)
        {
            printf("console> opening firefox\n");
            rhea::shell_runCommandNoWait("/usr/bin/firefox ~/GPU/test_data/test.html");
        }
        else if (strcasecmp(str,"chrome") == 0)
        {
            printf("console> opening chrome\n");
            rhea::shell_runCommandNoWait("/usr/bin/google-chrome ~/GPU/test_data/test.html");
        }
        else if (strcasecmp(str,"clientlist") == 0)
        {
            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_CLIENT_LIST, (u32)0);
        }

        //se inizia con 0x vuol dire che voglio parlare al client
        else if (strncmp(str,"0x", 2) == 0)
        {
            rhea::string::parser::Iter iter, result;
            iter.setup (str);

            if (rhea::string::parser::extractValue (iter, &result, " \n", 2))
            {
                char clientHex[32];
                result.copyCurStr (clientHex, sizeof(clientHex));
                //printf ("console> client hex=%s\n", clientHex);

                u32 clientHandleAsU32;
                if (rhea::string::convert::hexToInt (&clientHex[2], &clientHandleAsU32))
                {
                    HWebsokClient hClient;
                    hClient.initFromU32(clientHandleAsU32);

                    const char *everythingAfterClientHex = iter.getCurStrPointer();

                    //vediamo se c'è un comando
                    if (rhea::string::parser::extractValue (iter, &result, " \n", 2))
                    {
                        char clientCmd[128];
                        result.copyCurStr (clientCmd, sizeof(clientCmd));
                        //printf ("console> client cmd=%s\n", clientCmd);

                        if (strcasecmp(clientCmd,"ping") == 0)
                        {
                            printf ("console> sending ping to [0x%02X]\n", hClient.asU32());
                            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_PING, hClient.asU32());
                        }
                        else if (strcasecmp(clientCmd,"close") == 0)
                        {
                            printf ("console> sending close to [0x%02X]\n", hClient.asU32());
                            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_CLOSE, hClient.asU32());
                        }
                        else
                        {
                            rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_STRING, hClient.asU32(), everythingAfterClientHex, strlen(everythingAfterClientHex));
                        }
                    }
                }

            }


        }


    }
    printf ("console> fin\n");
    return 1;
}



/**************************************************************************
 * Risponde alle richieste di guiBrdige simulando la presenza di una GPU
 */
i16 gpuThreadFn (void *userParam)
{
    sConsoleInitParam *init = (sConsoleInitParam*)userParam;
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


    //creo un canale di comunicazione per il thread della console
    HThreadMsgR chConsoleR;
    HThreadMsgW chConsoleW;
    rhea::thread::createMsgQ ( &chConsoleR, &chConsoleW);

    //creo il thread della console
    rhea::HThread hConsole;
    {
        sConsoleInitParam    init;
        init.hRead = chConsoleR;
        init.hWriteToServer = hQMessageToWebserver;

        rhea::thread::create (&hConsole, consoleThreadFn, &init);
    }


    //creo un thread che si occupa di fare le veci della GPU, rispondendo alle eventuali richieste che arrivano da guibridge::server
    rhea::HThread hGPU;
    {
        sConsoleInitParam    init;
        init.hRead = hQMessageFromWebserver;
        init.hWriteToServer = hQMessageToWebserver;

        rhea::thread::create (&hGPU, gpuThreadFn, &init);
    }


    //attendo che il thread del server muoia
    rhea::thread::waitEnd(hServer);

    //attendo che il thread della console muoia
    rhea::thread::waitEnd(hConsole);


    //chiudo i canali di comunicazione
    rhea::thread::deleteMsgQ (chConsoleR, chConsoleW);


    rhea::deinit();
    return 0;
}

