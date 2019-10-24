#ifndef _rheaApp_h_
#define _rheaApp_h_
#include "../rheaCommonLib/Protocol/IProtocol.h"
#include "../rheaCommonLib/Protocol/IProtocolChannell.h"
#include "rheaAppEnumAndDefine.h"
#include "rheaAppFileTransfer.h"

namespace rhea
{
	namespace app
	{

		bool		decodeSokBridgeMessage (const u8 *bufferIN, u16 nBytesInBuffer, sDecodedMsg *out, u16 *out_nBytesConsumed);
					/*	decodifica un msg ricevuto da SockeBridge.
						Se in [buffer] c'è un messaggio valido, ritorna true, filla [out] e scrive in [out_nBytesConsumed] il num di bytes
						utilizzati per decodificare il msg.
						Se ritorna true, allora vuol dire che [out] contiene un valido messaggio.
						Il tipo di messaggio è indicato da out->what.
						A seconda del valore di out->what, fare riferimento a out->data.asEvent oppure out->data.asFileTransf

						Se esiste un payload (ie payloadLen > 0), allora il puntatore out->data.asXXX.payload punta direttamente al payload indicizzando la locazione
						di [buffer] alla quale inizia il payload stesso (non faccio una copia locale del payload quindi..)
					*/

		
		/******************************************************************************
		 *
		 *	registrazione iniziale a socket bridge
		 *
		 */
		bool		handleInitialRegistrationToSocketBridge (rhea::ISimpleLogger *log, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, const socketbridge::SokBridgeClientVer &version,
															 socketbridge::SokBridgeIDCode *out_idCode, u32 *out_SMUversion);
					/*	inizia e cerca di portare a termine tutte le necessarie procedure di registrazione a SocketBridge.
						Se ritorna true, allora [out_idCode] contiene un valido idCode ricevuto da SocketBridge e la connessione con SocketBridge è da ritenersi valida.
						Se ritorna false, qualcosa è andato storto.
						[log] è opzionale, passare NULL se non si desidera un output verboso
					*/
		void		send_requestIDCodeAfterConnection(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const socketbridge::SokBridgeClientVer &version);
		void		send_identifyAfterConnection(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const socketbridge::SokBridgeClientVer &version, socketbridge::SokBridgeIDCode &idCode);




		/******************************************************************************
		 *
		 *	comandi vari
		 *
		 */
		namespace CurrentCPUMessage
		{
			void	ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void	decodeAnswer (const sDecodedEventMsg &msg, u8 *out_msgImportanceLevel, u16 *out_msgLenInBytes, u8 *out_messageInUTF8, u32 sizeOfOutMessage);
					//ask		=> chiede a SocketBridge di mandare l'attuale messaggio di CPU
					//decodeAnswer	=> decodifica la risposta ricevuta
		}

		namespace CurrentCPUStatus
		{
			void		ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType);
						//ask			=> chiede a SocketBridge di mandare l'attuale stato della CPU
						//decodeAnswer	=> decodifica la risposta ricevuta
		}

		namespace CurrentSelectionRunningStatus
		{
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::eRunningSelStatus *out);
						//è socketbridge che spontaneamente invia questo messaggio, noi possiamo solo dedodificarlo, non possiamo chiederlo.
						//Il messaggio indica lo stato di "running" della selezione attualmente in esecuzione
		}

		namespace CurrentCredit
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, u8 *out_creditoInFormatoStringa, u32 sizeOfOut); //min 16 bytes needed
			//ask			=> chiede a SocketBridge di mandare l'attuale stato della CPU
			//decodeAnswer	=> decodifica la risposta ricevuta
		}
		
		namespace ClientList
		{
			void		ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, rhea::Allocator *allocator, u16 *out_nClientConnected, u16 *out_nClientInfo, rhea::DateTime *out_dtCPUBridgeStarted, socketbridge::sIdentifiedClientInfo **out_list);
			//ask_		=> chiede a SocketBridge di mandare la lista dei client connessi
			//decode_	=> decodifica la risposta ricevuta
		}

		namespace CurrentSelectionAvailability
		{
			void		ask (rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, u8 *out_numSel, u8 *out_selectionAvailability, u32 sizeOfSelecionAvailability);
			//ask_		=> chiede a SocketBridge di mandare lo stato di disponibilità di tutte le selezioni
			//decode_	=> decodifica la risposta ricevuta, [out_selectionAvailability] è un byte per ogni selezione, 0x00 vuol dire che la sel non è disponibili
			//				out_selectionAvailability[0] si riferisce alla selezione 1
		}

		namespace ButtonProgPressed
		{
			//void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg);
			//ask_		=> non è possibile chiedere lo stato del btnProg, è CPUBridge che invia le notifica alla bisogna
			//decode_	=> decodifica la risposta ricevuta
		}

		namespace CurrentCPUInitParam
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::sCPUParamIniziali *out);
			//ask_		=> chiede a SocketBridge i parametri iniziali inviati dalla CPU
			//decode_	=> decodifica la risposta ricevuta

		}

		namespace ReadDataAudit
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer (const sDecodedEventMsg &msg, cpubridge::eReadDataFileStatus *status, u16 *totKbSoFar, u16 *out_fileID);
			//ask_		=> chiede a SocketBridge il file di audit
			//decode_	=> il recupero del dataAudit avviene in step. Ad ogni step si riceve un messaggio la cui decodifica ci dice come siamo messi con il download
			//				Quando [status] == eReadDataFileStatus_finishedOK, vuol dire che è download è terminato con successo
						    
		}
			   
		namespace ReadVMCDataFile
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::eReadDataFileStatus *status, u16 *totKbSoFar, u16 *out_fileID);
			//ask_		=> chiede a SocketBridge il file da3
			//decode_	=> come sopra

		}

		namespace WriteLocalVMCDataFile
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, const char *localFilePathAndName);
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::eWriteDataFileStatus *status, u16 *totKbSoFar);
			//ask_		=> chiede a SocketBridge di installare un file da3 locale alla SMU
			//decode_	=> come sopra

		}

		namespace CurrentVMCDataFileTimestamp
		{
			void		ask(rhea::IProtocolChannell *ch, rhea::IProtocol *proto);
			void		decodeAnswer(const sDecodedEventMsg &msg, cpubridge::sCPUVMCDataFileTimeStamp *out);
			//ask_		=> chiede a SocketBridge il file da3
			//decode_	=> come sopra

		}


		/******************************************************************************
		 *
		 *	file transfer
		 *
		 *	In tutte le chiamata c'è il parametro [sendBuffer]. Questo è un buffer allocato da qualcuno che deve essere abbastanza grande da contenere l'intero messaggio.
		 *	In generale quindi, questo buffer deve essere grosso almeno [packetSize] + 32 bytes 
		 */
		 //vedi anche "rheaAppFileTransfer.h"
		namespace RawFileTrans
		{
			void	sendToSocketBridge (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u8 *sendBuffer, u32 sizeOfSendBufferIN, const void *optionalData, u16 sizeoOfOptionalData);
		}

	} // namespace app

} // namespace rhea


#endif // _rheaApp_h_

