#ifndef _CmdHandler_eventReqWriteLocalVMCDataFile_h_
#define _CmdHandler_eventReqWriteLocalVMCDataFile_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqWriteLocalVMCDataFile
     *
     *  Il client vuole uppare un DA3 sulla CPU.
     *  Il file DA3 in questione viene copiato dapprima nella cartella locale app/temp e poi inviato alla CPU.
     *  Quindi per questo comando è necessario che client e SocketBridge vivano e funziono sullo stesso PC in quanto devono avere accesso allo 
     *  stesso hard disk.
     *  E' sostanzialmente un comando di comodo usato dalla GPU (intesa come il prg c++ realizzato con Qt) per quando si vuole uppare
     *  un da3 locale.
     *  Non è da utilizzarsi per upload di file da remoto.
     *  In quest'ultimo caso, la sequenza corretta di operazioni sarebbe:
            1) il client remoto avvia un file transfer per trasferire il file DA3 sul PC dove risiede SocketBridge
            2) viene invocato questo comando con il nome del file trasferito 
     *
     */
    class CmdHandler_eventReqWriteLocalVMCDataFile : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqWriteLocalVMCDataFile;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS;
		
		CmdHandler_eventReqWriteLocalVMCDataFile(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqWriteLocalVMCDataFile_h_
