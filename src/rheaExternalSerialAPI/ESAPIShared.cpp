#include "ESAPIShared.h"
#include "ESAPI.h"
#include "ESAPIProtocol.h"

using namespace esapi;


//*****************************************
sShared::sShared()
{}

//*****************************************
sShared::eRetCode sShared::run(Module *module)
{
    retCode = sShared::RETCODE_UNKNOWN;
    while (retCode == sShared::RETCODE_UNKNOWN)
    {
        //qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
        //TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
#ifdef LINUX
        const u8 nEvents = shared->waitableGrp.wait(10000);
#else
        const u8 nEvents = waitableGrp.wait(100);
#endif
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case ESAPI_WAITABLEGRP_EVENT_FROM_SERVICE_MSGQ:
                        priv_handleMsgFromServiceQ(module);
                        break;

                    case ESAPI_WAITABLEGRP_EVENT_FROM_A_SUBSCRIBER:
				        //evento generato dalla msgQ di uno dei miei subscriber
                        {
                            OSEvent h = waitableGrp.getEventSrcAsOSEvent(i);
                            sSubscription *sub = subscriberList.findByOSEvent(h);
						    if (sub)
                                priv_handleMsgFromSubscriber(module, sub);
				        }
                        break;

                    case ESAPI_WAITABLEGRP_EVENT_FROM_CPUBRIDGE:
                        {
                            rhea::thread::sMsg msg;
                            while (rhea::thread::popMsg (cpuBridgeSubscriber.hFromMeToSubscriberR, &msg))
                            {
                                const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
                                if (!protocol->onMsgFromCPUBridge (cpuBridgeSubscriber, msg, handlerID))
                                    module->virt_handleMsgFromCPUBridge(this, cpuBridgeSubscriber, msg, handlerID);
                                rhea::thread::deleteMsg(msg);
                            }
                        }
                        break;

                    default:
						DBGBREAK;
                        logger->log("esapi::sShared::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			case OSWaitableGrp::evt_origin_socket:
				{
					//Ho ricevuto dei dati lungo una socket
					OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
                    module->virt_handleMsgFromSocket (this, sok, clientUID);
				}
				break;

#ifdef LINUX
            case OSWaitableGrp::evt_origin_serialPort:
                priv_handleRS232(module);
                break;
#endif

			default:
                logger->log("esapi::sShared::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

#ifndef LINUX
        priv_handleRS232(module);
#endif
    }

    return retCode;
}


//*********************************************************
void sShared::priv_handleMsgFromServiceQ(Module *module)
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (serviceMsgQR, &msg))
    {
        switch (msg.what)
        {
        default:
            module->virt_handleMsgFromServiceQ(this, msg);
            break;

        case ESAPI_SERVICECH_SUBSCRIPTION_REQUEST:
            {
                sSubscription *sub = subscriberList.onSubscriptionRequest (localAllocator, logger, msg);
                waitableGrp.addEvent(sub->hEvent, ESAPI_WAITABLEGRP_EVENT_FROM_A_SUBSCRIBER);
            }
			break;
		}
		
		rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void sShared::priv_handleMsgFromSubscriber(Module *module, sSubscription *sub)
{
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
    {
        const u16 handlerID = (u16)msg.paramU32;

        switch (msg.what)
        {
        default:
            module->virt_handleMsgFromSubscriber(this, *sub, msg, handlerID);
            break;

        case ESAPI_ASK_UNSUBSCRIBE:
            rhea::thread::deleteMsg(msg);
            waitableGrp.removeEvent (sub->hEvent);
            subscriberList.unsubscribe (localAllocator, sub);
            return;

        case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
            notify_MODULE_TYPE_AND_VER (sub->q, handlerID, logger, eExternalModuleType_none, esapi::Protocol::ESAPI_VERSION_MAJOR, esapi::Protocol::ESAPI_VERSION_MINOR);
            break;
        }

        rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void sShared::priv_handleRS232 (Module *module)
{
    //protocol gestisce tutti i comandi canonici, ad esclusione dei comandi 'R'
    if (protocol->rs232_read())
        return;

    //ho un comando 'R' in canna, deve gestirlo il modulo attualmente in carico
	module->virt_handleMsg_R_fromRs232 (this, protocol->rsr232_getBufferIN());
}