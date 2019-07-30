#include "../rheaGUIBridge/GUIBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "Client.h"


struct sConsoleInitParam
{
    HThreadMsgR hRead;
    HThreadMsgW hWriteToServer;
};

rhea::StdoutLogger  log;

//**************************************************************************
i16 consoleThreadFn (void *userParam)
{
    sConsoleInitParam *init = (sConsoleInitParam*)userParam;
    //HThreadMsgR chRead = init->hRead;
    HThreadMsgW chWriteToServer = init->hWriteToServer;


    printf ("> starting...\n");
    printf ("> type quit to terminate\n");
    while (1)
    {
        char str[64];
        printf (">");

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
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}


//*****************************************************
bool openSocket (OSSocket *sok, const char *ip, u32 port)
{
    log.log ("opening socket [%s:%d]: ", ip, port);
    eSocketError sokErr = OSSocket_openAsTCPClient (sok, ip, port);
    if (sokErr != eSocketError_none)
    {
        log.log ("FAIL, error code=%d\n", sokErr);
        return false;
    }
    log.log ("OK\n");
    return true;
}


//*****************************************************
void waitAnswer (Client &client)
{}


/*****************************************************
 * true se quit
 */
bool processCommand (Client &client, const char *str)
{
    if (strcasecmp(str,"quit") == 0)
    {
        client.sendClose();
        log.log ("quitting...\n");
        return true;
    }
    if (strcasecmp(str,"ping") == 0)
    {
        client.sendPing();
        return true;
    }
    else if (strcasecmp(str,"clientlist") == 0)
    {
        //rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_CLIENT_LIST, (u32)0);
        return false;
    }

    /*se inizia con 0x vuol dire che voglio parlare al client
    else if (strncmp(str,"0x", 2) == 0)
    {
        rhea::string::parser::Iter iter, result;
        iter.setup (str);

        if (rhea::string::parser::extractValue (iter, &result, " \n", 2))
        {
            char clientHex[32];
            result.copyCurStr (clientHex, sizeof(clientHex));
            //printf ("> client hex=%s\n", clientHex);

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
                    //printf ("> client cmd=%s\n", clientCmd);

                    if (strcasecmp(clientCmd,"ping") == 0)
                    {
                        printf ("> sending ping to [0x%02X]\n", hClient.asU32());
                        rhea::thread::pushMsg (chWriteToServer, GUIBRIDGE_CONSOLE_EVENT_PING, hClient.asU32());
                    }
                    else if (strcasecmp(clientCmd,"close") == 0)
                    {
                        printf ("> sending close to [0x%02X]\n", hClient.asU32());
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
    */

    log.log ("unknown command\n");
    return false;
}

//*****************************************************
void handleInput (Client &client)
{
    bool bQuit = false;
    while ( bQuit == false)
    {
        char str[64];
        log.log (">");

        int c;
        u16 ct=0;
        str[0] = 0x00;
        while ( (c = getch()) )
        {
            log.log ("%c", (char)c);

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

        log.incIndent();
        bQuit = processCommand(client, str);
        log.decIndent();
    }
}

//*****************************************************
int main()
{
    const char IP[] = {"127.0.0.1"};
    const int PORT_NUMBER = 2280;
    OSSocket sok;

    rhea::init(NULL);

    log.log ("==============================================\n");
    log.log ("RHEAConsole, version 1.0\n");
    log.log ("==============================================\n");

    log.log ("connecting...\n");
    log.incIndent();
        if (!openSocket (&sok,IP, PORT_NUMBER))
            return 0;

        log.log ("sending handshake: ");
        if (!rhea::ProtocolConsole::client_handshake(sok, &log))
        {
            log.log ("FAIL\n");
            OSSocket_close(sok);
            return 0;
        }
        log.log ("OK\n");
        log.log ("ready, type quit to terminate\n\n");
    log.decIndent();


    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    {
        Client *client = RHEANEW(allocator, Client)(allocator, sok);
        handleInput (*client);
        RHEADELETE(allocator, client);
    }


    log.log ("closing socket\n");
    OSSocket_close (sok);

    log.log ("\n\nFIN\n\n");
    rhea::deinit();
    return 0;
}

