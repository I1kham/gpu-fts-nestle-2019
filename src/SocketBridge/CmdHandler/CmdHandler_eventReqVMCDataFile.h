#ifndef _CmdHandler_eventReqVMCDataFile_h_
#define _CmdHandler_eventReqVMCDataFile_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqVMCDataFile
     *
     *  Il client vuole leggere il DA3.
     *  La richiesta viene passata a CPUBridge la quale scarica il DA3 dalla CPU e lo mette in una cartella locale con un nome
     *  ben preciso (app/temp/vmcDataFile%d.da3)
     *  Il [%d] è un numero che viene riportato nella notifica di risposta.
     *  Durante il download, CPUBridge invia notifiche spontanee di tipo CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS a tutti i client
     *  con indicazioni circa lo stato di avanzamento del download.
     *  A fine download, il client che ha iniziato la richiesta riceve una ulteriore notifica ad indicare l'esito finale dell'operazione.
     *
     *  E' importante notare che il DA3 non viene trasferito al client remoto, viene solo copiato in una cartella locale a SocketBridge.
     *  Quando il client remoto riceve la notifica che il download è terminato con successo, allora può far partire una richiesta di
     *  file transfer indicando come target il DA3 appena scaricato localmente (identidicabile da [fileID] che è uno dei parametri
     *  compresi nella notifica inviata da CPUBridge a fine download
     *
     */
    class CmdHandler_eventReqVMCDataFile : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqVMCDataFile;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS;
		
					CmdHandler_eventReqVMCDataFile(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqVMCDataFile_h_
