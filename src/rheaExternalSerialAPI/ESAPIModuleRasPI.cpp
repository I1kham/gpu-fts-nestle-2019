#include "ESAPIModuleRasPI.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../CPUBridge/CPUBridge.h"

using namespace esapi;

//********************************************************
ModuleRasPI::ModuleRasPI()
{

}

//********************************************************
bool ModuleRasPI::setup ( sGlob *glob)
{
    this->glob = glob;

    //buffer
	rs232BufferIN.alloc (glob->localAllocator, SIZE_OF_RS232BUFFERIN);
	rs232BufferOUT = (u8*)RHEAALLOC(glob->localAllocator, SIZE_OF_RS232BUFFEROUT);

    //aggiungo la serviceMsgQ alla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.addEvent(h, WAITLIST_EVENT_FROM_SERVICE_MSGQ);
    }

    //aggiungo i subscriber alla wait list
    for (u32 i=0; i<glob->subscriberList.list.getNElem();i++)
    {
        waitableGrp.addEvent(glob->subscriberList.list(i)->hEvent, WAITLIST_EVENT_FROM_A_SUBSCRIBER);
    }

	return true;
}

//********************************************************
void ModuleRasPI::priv_unsetup()
{
    //rimuovo la serviceMsgQ dalla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.removeEvent(h);
    }

    rs232BufferIN.free (glob->localAllocator);
    RHEAFREE(glob->localAllocator, rs232BufferOUT);
}

//*********************************************************
void ModuleRasPI::priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend)
{
    rhea::rs232::writeBuffer (glob->com, buffer, numBytesToSend);
}

//********************************************************
eExternalModuleType ModuleRasPI::run()
{
    glob->logger->log ("ModuleRasPI:: now in BOOT mode...\n");
    glob->logger->incIndent();
    priv_boot_run();
    glob->logger->log ("FIN\n");
    glob->logger->decIndent();

    glob->logger->log ("ModuleRasPI:: now in RUNNING mode...\n");
    glob->logger->incIndent();
    glob->logger->log ("FIN\n");
    glob->logger->decIndent();

    priv_unsetup();
    return esapi::eExternalModuleType_unknown;
}

//*********************************************************
void ModuleRasPI::priv_handleMsgFromServiceQ()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (glob->serviceMsgQR, &msg))
    {
        switch (msg.what)
        {
        default:
            DBGBREAK;
            break;

        case ESAPI_SERVICECH_SUBSCRIPTION_REQUEST:
            {
                sSubscription *sub = glob->subscriberList.onSubscriptionRequest (glob->localAllocator, glob->logger, msg);
                waitableGrp.addEvent(sub->hEvent, WAITLIST_EVENT_FROM_A_SUBSCRIBER);
            }
			break;
		}
		
		rhea::thread::deleteMsg(msg);
    }
}


/********************************************************
 * In questa fase, il modulo rasPI è slave, ovvero attende comandi da me via seriale; io a mia volta attendo comandi via subscriber.
 * Questa fase termina quando spedisco il comando # R 0x01 [ck] che manda il modulo rasPI
 * nella modaliù operativa vera e propria.
 * I comandi, io li mando se qualche subscriber mi dice di farlo..
 */
void ModuleRasPI::priv_boot_run()
{
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(10);
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case WAITLIST_EVENT_FROM_SERVICE_MSGQ:
                        priv_handleMsgFromServiceQ();
                        break;

                    case WAITLIST_EVENT_FROM_A_SUBSCRIBER:
				        //evento generato dalla msgQ di uno dei miei subscriber
                        {
                            OSEvent h = waitableGrp.getEventSrcAsOSEvent(i);
                            sSubscription *sub = glob->subscriberList.findByOSEvent(h);
						    if (sub)
                                priv_boot_handleMsgFromSubscriber(sub);
				        }
                        break;

                    default:
						DBGBREAK;
                        glob->logger->log("esapi::ModuleRasPI::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			default:
                glob->logger->log("esapi::ModuleRasPI::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}
    }
}

//*********************************************************
void ModuleRasPI::priv_boot_handleMsgFromSubscriber(sSubscription *sub)
{
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
    {
        const u16 handlerID = (u16)msg.paramU32;

        switch (msg.what)
        {
        default:
            glob->logger->log("ModuleRasPI::priv_boot_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
            break;

        case ESAPI_ASK_UNSUBSCRIBE:
            rhea::thread::deleteMsg(msg);
            waitableGrp.removeEvent (sub->hEvent);
            glob->subscriberList.unsubscribe (glob->localAllocator, sub);
            return;

        case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
            notify_MODULE_TYPE_AND_VER (sub->q, handlerID, glob->logger, glob->moduleInfo.type, glob->moduleInfo.verMajor, glob->moduleInfo.verMinor);
            break;

        case ESAPI_ASK_RASPI_START:
            //dico al rasPI di "startare"
            {
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x01;
                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, 3);
                ct++;
                priv_rs232_sendBuffer (rs232BufferOUT, ct);
                if (priv_boot_waitAnswer('R', 0x01, 4, 0, rs232BufferOUT, 1000))
                    notify_RASPI_STARTED(sub->q, handlerID, glob->logger);
            }
            break;

        case ESAPI_ASK_RASPI_GET_IPandSSID:
            //chiedo al rasPI IP e SSID
            {
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x02;
                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, 3);
                ct++;
                priv_rs232_sendBuffer (rs232BufferOUT, ct);

                //la risposta è a lunghezza variabile
                //# R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
                if (priv_boot_waitAnswer('R', 0x02, 9, 8, rs232BufferOUT, 1000))
                {
                    const char *ssid = (const char*)&rs232BufferOUT[8];
                    rs232BufferOUT[8 + rs232BufferOUT[7]] = 0x00;
                    notify_RASPI_WIFI_IPandSSID (sub->q, handlerID, glob->logger, rs232BufferOUT[3], rs232BufferOUT[4], rs232BufferOUT[5], rs232BufferOUT[6], ssid);
                }
            }
            break;
        }

        rhea::thread::deleteMsg(msg);
    }
}

//*******************************************************
bool ModuleRasPI::priv_boot_waitAnswer(u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	u8 ct = 0;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u8 ch;
		if (!rhea::rs232::readBuffer(glob->com, &ch, 1))
		{
			rhea::thread::sleepMSec(10);
			continue;
		}
		
		if (ct == 0)
		{
			if (ch == '#')
				answerBuffer[ct++] = ch;
		}
		else if (ct == 1)
		{
			if (ch == command)
				answerBuffer[ct++] = ch;
			else
				ct = 0;
		}
		else if (ct == 2)
		{
			if (ch == code)
				answerBuffer[ct++] = ch;
			else
				ct = 0;
		}
		else
		{
			answerBuffer[ct++] = ch;

            if (0 == whichByteContainsAdditionMsgLen)
            {
                if (ct == fixedMsgLen)
                {
                    if (rhea::utils::simpleChecksum8_calc(answerBuffer, fixedMsgLen - 1) == answerBuffer[fixedMsgLen - 1])
                        return true;
                    ct = 0;
                }
            }
            else
            {
                //questo caso vuol dire che il messaggio è lungo [fixedMsgLen] + quanto indicato dal byte [whichByteContainsAdditionMsgLen]
                if (ct >= whichByteContainsAdditionMsgLen)
                {
                    const u8 totalMsgSize = answerBuffer[whichByteContainsAdditionMsgLen] + fixedMsgLen;
                    if (ct == totalMsgSize)
                    {
                        if (rhea::utils::simpleChecksum8_calc(answerBuffer, totalMsgSize - 1) == answerBuffer[totalMsgSize - 1])
                            return true;
                        ct = 0;
                    }
                }

            }
		}
	}
	return false;
}



