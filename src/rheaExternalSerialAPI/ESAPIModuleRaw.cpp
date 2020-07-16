#include "ESAPIModuleRaw.h"
#include "ESAPI.h"
#include "ESAPIProtocol.h"


using namespace esapi;

//********************************************************
ModuleRaw::ModuleRaw()
{
}

//*********************************************************
void ModuleRaw::virt_handleMsgFromServiceQ	(sShared *shared, const rhea::thread::sMsg &msg)
{
    //non c'è nulla che questo modulo debba gestire in caso di messaggi ricevuti da altri thread sulla msgQ
    DBGBREAK;
}

//*********************************************************
void ModuleRaw::virt_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID)
{
    shared->logger->log("esapi::ModuleRaw::virt_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
}

//*********************************************************
void ModuleRaw::virt_handleMsgFromCPUBridge	(sShared *shared, cpubridge::sSubscriber &sub, const rhea::thread::sMsg &msg, u16 handlerID)
{
    //non c'è nulla da fare, tutte le notifiche utili in arrivo da CPUBridge sono già gestite da protocol
}


//*********************************************************
void ModuleRaw::virt_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam)
{
    //non dovrebbe mai accadere
    DBGBREAK;
}

/********************************************************
 * in buffer, ho un comando R da gestire (diverso da R1)
 */
void ModuleRaw::virt_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b)
{
    const u8 COMMAND_CHAR = 'R';
    assert(b->numBytesInBuffer >= 3 && b->buffer[0] == '#' && b->buffer[1] == COMMAND_CHAR);
	const u8 commandCode = b->buffer[2];

	switch (commandCode)
	{
	default:
        DBGBREAK;
        shared->logger->log ("esapi::ModuleRaw::virt_handleMsg_R_fromRs232() => ERR, invalid #R command [# R 0x%02X]\n", commandCode);
        b->removeFirstNBytes(1);
        break;

	case '1':
		//External module identify
        //C'è un modulo esterno, collegato alla seriale che vuole identificarsi
        //ricevuto: # R 1 [moduleType] [verMajor] [verMinor] [ck]
        //rispondo: # R 1 [result] [ck]
		{
            //parse del messaggio
            bool bValidCk = false;
            sESAPIModule moduleInfo;
            const u32 MSG_LEN = esapi::buildMsg_R1_externalModuleIdentify_parseAsk (b->buffer, b->numBytesInBuffer, &bValidCk, &moduleInfo.type, &moduleInfo.verMajor, &moduleInfo.verMinor);
            if (0 == MSG_LEN)
                return;
            if (!bValidCk)
            {
                b->removeFirstNBytes(2);
                return;
            }

            //rimuovo il msg dal buffer
            b->removeFirstNBytes(MSG_LEN);

           //rispondo via seriale confermando di aver ricevuto il msg
            u8 result = 0x00;
			switch (moduleInfo.type)
			{
			case eExternalModuleType_rasPI_wifi_REST:
				result = 0x01;
                shared->moduleInfo.type = moduleInfo.type;
                shared->moduleInfo.verMajor = moduleInfo.verMajor;
                shared->moduleInfo.verMinor = moduleInfo.verMinor;
                shared->retCode = sShared::RETCODE_START_MODULE_RASPI;
				break;

			default:
				result = 0x00;
				break;
			}

            u8 answer[16];
            const u32 n = esapi::buildMsg_R1_externalModuleIdentify_resp (result, answer, sizeof(answer));
            shared->protocol->rs232_write (answer, n);
		}
		break;
	}
}
