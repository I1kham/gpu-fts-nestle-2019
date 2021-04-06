#include "ESAPIModuleRaw.h"
#include "ESAPI.h"
#include "ESAPIProtocol.h"
#include "../rheaCommonLib/rheaUtils.h"


using namespace esapi;

//********************************************************
ModuleRaw::ModuleRaw()
{
}

//********************************************************
bool ModuleRaw::setup (sShared *shared)
{
    shared->moduleInfo.type = esapi::eExternalModuleType::none;
    shared->moduleInfo.verMajor = esapi::Protocol::ESAPI_VERSION_MAJOR;
    shared->moduleInfo.verMinor = esapi::Protocol::ESAPI_VERSION_MINOR;
    return true;
}


//*********************************************************
void ModuleRaw::virt_handleMsgFromServiceQ	(sShared *shared UNUSED_PARAM, const rhea::thread::sMsg &msg UNUSED_PARAM)
{
    //non c'è nulla che questo modulo debba gestire in caso di messaggi ricevuti da altri thread sulla msgQ
    DBGBREAK;
}

//*********************************************************
void ModuleRaw::virt_handleMsgFromSubscriber(sShared *shared, sSubscription &sub UNUSED_PARAM, const rhea::thread::sMsg &msg, u16 handlerID UNUSED_PARAM)
{
    shared->logger->log("esapi::ModuleRaw::virt_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
}

//*********************************************************
void ModuleRaw::virt_handleMsgFromCPUBridge	(sShared *shared UNUSED_PARAM, cpubridge::sSubscriber &sub UNUSED_PARAM, const rhea::thread::sMsg &msg UNUSED_PARAM, u16 handlerID UNUSED_PARAM)
{
    //non c'è nulla da fare, tutte le notifiche utili in arrivo da CPUBridge sono già gestite da protocol
}


//*********************************************************
void ModuleRaw::virt_handleMsgFromSocket (sShared *shared UNUSED_PARAM, OSSocket &sok UNUSED_PARAM, u32 userParam UNUSED_PARAM)
{
    //non dovrebbe mai accadere
    DBGBREAK;
}

/********************************************************
 * in buffer, ho un comando R da gestire (diverso da R1)
 */
void ModuleRaw::virt_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b)
{
    assert(b->numBytesInBuffer >= 4 && b->buffer[0] == '#' && b->buffer[1] == 'R');
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
			if (b->numBytesInBuffer < 7)	//devo avere almeno 7 char nel buffer
				return;

			if (b->buffer[6] != rhea::utils::simpleChecksum8_calc (b->buffer, 6))
			{
				b->removeFirstNBytes(2);
				return;
			}

			//parse del messaggio
			sESAPIModule moduleInfo;
			moduleInfo.type = (eExternalModuleType)b->buffer[3];
			moduleInfo.verMajor = b->buffer[4];
			moduleInfo.verMinor = b->buffer[5];
            
			//rimuovo il msg dal buffer
			b->removeFirstNBytes(7);
			
           //rispondo via seriale confermando di aver ricevuto il msg
            u8 result = 0x00;
			switch (moduleInfo.type)
			{
			case eExternalModuleType::rasPI_wifi_REST:
				result = 0x01;
                shared->moduleInfo.type = moduleInfo.type;
                shared->moduleInfo.verMajor = moduleInfo.verMajor;
                shared->moduleInfo.verMinor = moduleInfo.verMinor;
                shared->retCode = sShared::eRetCode::START_MODULE_RASPI;
				break;

			default:
				result = 0x00;
				break;
			}

            u8 answer[16];
            const u32 n = esapi::buildAnswer ('R', '1', &result, 1, answer, sizeof(answer));
            shared->protocol->rs232_write (answer, n);
		}
		break;
	}
}
