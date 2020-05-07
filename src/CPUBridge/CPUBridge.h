/******************************************************************************************************************************
	CPUBridge � un thread che si occupa di astrarre la CPU vera e propria.
	CPUBridge riceve richieste attraverso una msgQ thread-safe (richiesta che arrivano da SocketBridge) ed eventualmente le
	traduce e le passa alla CPU vera e propria.
	Si occupa anche di gestire le risposte in arrivo dalla CPU ed eventualmente comunicare a SocketBridge informazioni rilevanti.
	Ad esempio, CPUBridge in maniera del tutto trasparente e all'insaputa di SocketBridge, invia periodicamente una richiesta di stato alla CPU.
	La risposta a questa richiesta (o anche la non risposta) pu� generare un messaggio da CPUBridge verso SocketBridge per esempio per informare che lo 
	stato delle selezioni � cambiato, oppure che lo stato della CPU � cambiato.
	CPUBridge notifica SocketBridge solo quanto "qualche cosa" nella CPU vera e propria � cambiata.
	Per esempio, CPUBridge non manda 100 messaggi a SocketBridge ribadendo costantemente che lo stato della CPU � "disponibile". Gli manda un solo
	messaggio e, fino a che lo stato di CPU non cambia, non manda altri messaggi del genere (a meno che non sia stato esplicitamente richiesto
	da SocketBridge). Al variare dello stato di CPU, allora CPUBrdige invia una notifica "spontanea" a SocketBridge informandolo della variazione
	di stato.

	Il canale di comunicazione tra CPUBridge e la CPU vera e propria � astratto tramite la classe CPUChannel.
	Al momento esistono 2 classi concrete derivate da CPUChannel:
		CPUChannelCom	=> invia e riceve messaggi tramite la porta seriale
		CPUChannelFake	=> finge di inviare messaggi ad una ipotetica CPU e fornisce riposte (in pratica � un simulatore di CPU da usarsi per applicazioni
							stand alone come il rheaMedia)
	Una istanza concreta del canale di comunicazione da utilizzare viene passato come parametro nella funzione di inizializzazione del thread,
	vedi cpubridge::startServer()

	CPUBridge riceve richieste da altri thread (generalmente solo da SocketBridge, ma i thread in ingresso possono essere N).
	Un generico thread (es: SocketBridge) si "subscribe()" a CPUBridge in modo da poter comunicare e ricevere messaggi (vedi cpubridge::subscribe())
	Un volta iscritto, il thread invia richieste tramite l'apposita msgQ fornita durante la subscribe() e riceve eventuali risposte/notifiche sulla stessa
	msgQ.

	In caso di notifiche spontanee, tutti i thread iscritti a CPUBridge ricevono la notifica automaticamente.
	Ad esempio, quando CPU cambia di stato, o quando cambia la disponibilit� delle selezioni, CPUBrdige invia spontaneamente una notifica a tutti i
	thread che si sono iscritti.

	Le notifiche di CPUBridge verso i thread iscritti avvengono tramite la chiamata ai vari metodi cpubridge::notify_xxx (vedi in questo file .h)
	Una generica funzione cpubridge::notify_xxx() solitamente non fa altro che pushare un messaggio sulla msgQ del thread ricevente.
	Come esempio, consideriamo la fn cpubridge:::notify_CPU_STATE_CHANGED(...)
		Quando CPUBridge invoca questa fn, sulla msgQ del thread ricevente viene pushato un messaggio il cui "what" � == CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED
		L'elenco delle define delle notifiche si trova in CPUBridgeEnumAndDefine.h. 
		Tutte le define di questo tipo iniziano con CPUBRIDGE_NOTIFY_

		Quando il thread riceve una notifica, deve chiamare una fn per tradurre il messaggio ricevuto, ovvero per ottenere in maniera "safe" gli eventuali
		parametri che il messaggio porta con se.
		Le funzioni di traduzione si chiamano tutte cpubridge::translateNotify_xxx
		Ad esempio, la fn di traduzione per cpubridge:::notify_CPU_STATE_CHANGED(...) si chiama translateNotify_CPU_STATE_CHANGED(..).
		La funzione di traduzione non � strettamente obbligatoria, ma � buona norma associare ad ogni notify() la sua translate() in modo da astrarre
		il modo in cui i parametri sono impacchettati nel messaggio. In questo modo, se in futuro si vuole aggiungere/togliere un parametro da una
		notify() esistente, basta modificare la sua translate() e ricompilare senza dover andare a cercare in tutto il codice dove e quando quella
		notify veniva utilizzata e tradotta.


	In maniera molto simile, quando un thread vuol chiedere qualcosa a CPUBridge, usa una delle funziona cpubridge::ask_ (vedi a fine di questo .h)
	La maggior parte delle funzioni cpubridge::ask_ non prevedono parametri ma, qualora lo facessero, � bene implementare la relativa 
	cpubridge::translate_ per gli stessi motivi esposti poco sopra.
	Ad esempio:
		cpubbrige::ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber);
		cpubbrige::translate_CPU_START_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNumber);

		Il thread informa CPUBridge che si vuole far partire la selezione [selNumber] usando la fn cpubbrige::ask_CPU_START_SELECTION()
		CPUBridge riceve questo messaggio sulla sua msgQ e lo traduce per recuperare il parametro [selNumber].
		CPUBridge quindi chiama la fn cpubbrige::translate_CPU_START_SELECTION() per tradurre il msg ricevuto.
		Dopo la traduzione, CPUBridge invia una richiesta alla vera CPU per iniziare la selezione.
		A seguito di questa operazione, verosimilmente CPUBridge comincer� ad inviare una serie di notifiche spontanee a tutti i thread iscritti per
		esempio per indicare che la CPU � passata da stato "disponibile" a stato "erogazione in corso" e, successivamente, da "erogazione in corso" di 
		nuovo in "disponibile".

	
*/
#ifndef _CPUBridge_h_
#define _CPUBridge_h_
#include "CPUBridgeEnumAndDefine.h"
#include "CPUChannel.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"


namespace cpubridge
{
	bool        startServer (CPUChannel *chToCPU, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread, HThreadMsgW *out_hServiceChannelW);
				/*
					Ritorna false in caso di problemi.
					Se ritorna true, allora:
						[out_hThread]				� l'handle del thread che � stato creato
						[out_hServiceChannelW]		� il canale di comunicazione di "servizio" da utilizzarsi per richieste speciali (tipo subsribe)
				*/

	void		subscribe (const HThreadMsgW &hServiceChannelW, const HThreadMsgW &hAnswerHere);
					/*	Qualcuno vuole iscriversi alla coda di messaggi di output di CPUBridge.
						CPUBridge invier� la risposta a questa richiesta sul canale identificato da [hAnswerHere].

						Il thread richiedente deve quindi monitorare la propria msgQ in attesa di un msg di tipo CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER e tradurlo con
						translate_SUBSCRIPTION_ANSWER() la quale filla la struttura sCPUSubscriber da usare poi per le comunicazioni e il monitoring dei messaggi
					*/
	void		translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, sSubscriber *out, u8 *out_cpuBridgeVersion);

	void		unsubscribe (const sSubscriber &sub);

    void        loadVMCDataFileTimeStamp (sCPUVMCDataFileTimeStamp *out);
    bool        saveVMCDataFileTimeStamp(const sCPUVMCDataFileTimeStamp &ts);
	
	/***********************************************
		buildMsg_xxxx
			ritornano 0 se out_buffer non � abbastanza grande da contenere il messaggio.
			altrimenti ritornano il num di byte inseriti in out_buffer
	*/
	u8			buildMsg_checkStatus_B (u8 keyPressed, u8 langErrorCode, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_initialParam_C (u8 gpuVersionMajor, u8 gpuVersionMinor, u8 gpuVersionBuild, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_restart_U (u8 *out_buffer, u8 sizeOfOutBuffer);
    u8			buildMsg_readDataAudit (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_readVMCDataFile(u8 blockNum, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_writeVMCDataFile (const u8 *buffer64yteLettiDalFile, u8 blockNum, u8 totNumBlocks, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getVMCDataFileTimeStamp (u8 *out_buffer, u8 sizeOfOutBuffer);
    u8			buildMsg_Programming (eCPUProgrammingCommand cmd, const u8 *optionalData, u32 sizeOfOptionalData, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getExtendedConfigInfo (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_writePartialVMCDataFile (const u8 *buffer64byte,  u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset, u8 *out_buffer, u8 sizeOfOutBuffer);
					/* se voglio inviare i blocchi 3, 6, 10, 12 alla cpu, invio 4 messaggi:
						blocco 1 di 4, offset=3
						blocco 2 di 4, offset=6
						blocco 3 di 4, offset=10
						blocco 4 di 4, offset=12

						[blockNumOffset] parte da 0, [blocco_n_di] parte da 1
					*/
	u8			buildMsg_setDecounter  (eCPUProgrammingCommand_decounter which, u16 valore, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getAllDecounterValues (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_attivazioneMotore(u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getStatoCalcoloImpulsiGruppo (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_setFattoreCalibMotore (eCPUProgrammingCommand_motor motore, u16 valoreInGr, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getStatoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_calcolaImpulsiGruppo (u8 macina_1o2, u16 totalePesata_dGrammi, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getTime (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getDate(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_setTime(u8 *out_buffer, u8 sizeOfOutBuffer, u8 hh, u8 mm, u8 ss);
	u8			buildMsg_setDate(u8 *out_buffer, u8 sizeOfOutBuffer, u16 year, u8 month, u8 day);
	u8			buildMsg_getPosizioneMacina (u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1o2);
	u8			buildMsg_setMotoreMacina (u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m);
	u8			buildMsg_testSelection (u8 *out_buffer, u8 sizeOfOutBuffer, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d);
	u8			buildMsg_getNomiLingueCPU (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_disintallazione(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_ricaricaFasciaOrariaFreevend (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_EVAresetPartial(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getVoltAndTemp (u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getCPUOFFReportDetails(u8 *out_buffer, u8 sizeOfOutBuffer, u8 indexNum);
					//indexNum >=0 <=19
	u8			buildMsg_getLastFluxInformation(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getCPUStringVersionAndModel(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_startModemTest(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_EVAresetTotals(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getTimeNextLavaggioSanCappuccinatore(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_startTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getStatoTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_startTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getStatoTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer);


	/***********************************************
		notify_xxxx
			il thread CPUBridge pusha questi messaggi sulle coda di uscita dei thread che si sono "subscribed" ogni volta che qualche evento importante accade oppure in risposta
			a specifiche richieste.
	*/
	void		notify_CPUBRIDGE_DYING (const sSubscriber &to);
					/*	notifica inviata a tutti i subscriber quando CPUBridge sta per morire. Il free della struttura sSubscriber � compito di CPUBridge; il subscriber deve semplicemente
						prendere atto del fatto che CPUBridge � morto ed agire di conseguenza
					*/

	void		notify_CPU_STATE_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType, u16 flag1);
	void		translateNotify_CPU_STATE_CHANGED (const rhea::thread::sMsg &msg, eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType, u16 *out_flag1);

	void		notify_CPU_NEW_LCD_MESSAGE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPULCDMessage *msg);
	void		translateNotify_CPU_NEW_LCD_MESSAGE(const rhea::thread::sMsg &msg, sCPULCDMessage *out_msg);

	void		notify_CPU_CREDIT_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const void *credit, u8 sizeOfCredit);
	void		translateNotify_CPU_CREDIT_CHANGED (const rhea::thread::sMsg &msg, u8 *out_credit, u16 sizeOfOut);
	
	void		notify_CPU_SEL_AVAIL_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUSelAvailability *s);
	void		translateNotify_CPU_SEL_AVAIL_CHANGED (const rhea::thread::sMsg &msg, sCPUSelAvailability *out);

	void		notify_CPU_SEL_PRICES_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 numPrices, u8 numDecimals, const u16 *prices);
	void		translateNotify_CPU_SEL_PRICES_CHANGED(const rhea::thread::sMsg &msg, u8 *out_numPrices, u8 *out_numDecimals, u16 *out_prices);
					//out_prices deve essere di almeno NUM_MAX_SELECTIONS elementi

	void		notify_CPU_RUNNING_SEL_STATUS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eRunningSelStatus s);
	void		translateNotify_CPU_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg, eRunningSelStatus *out_s);

	void		notify_CPU_FULLSTATE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUStatus *s);
	void		translateNotify_CPU_FULLSTATE(const rhea::thread::sMsg &msg, sCPUStatus *out_s);
	
	void		notify_CPU_INI_PARAM (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUParamIniziali *s);
	void		translateNotify_CPU_INI_PARAM(const rhea::thread::sMsg &msg, sCPUParamIniziali *out_s);

    void		notify_CPU_BTN_PROG_PRESSED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);
    void		translateNotify_CPU_BTN_PROG_PRESSED (const rhea::thread::sMsg &msg);

    void		notify_READ_DATA_AUDIT_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID);
    void		translateNotify_READ_DATA_AUDIT_PROGRESS (const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID);
					/* fileID � un numero che viene appeso al nome del file durante lo scaricamento.
						Posto che il download vada a buon fine, il file localmente si trova in app/temp/dataAudit[FILE_ID].txt (es app/temp/dataAudit5.txt
					*/

	void		notify_READ_VMCDATAFILE_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID);
	void		translateNotify_READ_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID);
					/* fileID � un numero che viene appeso al nome del file durante lo scaricamento.
						Posto che il download vada a buon fine, il file localmente si trova in app/temp/vmcDataFile[FILE_ID].da3 (es app/temp/vmcDataFile7.da3
					*/

	void		notify_WRITE_VMCDATAFILE_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eWriteDataFileStatus status, u16 totKbSoFar);
	void		translateNotify_WRITE_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eWriteDataFileStatus *out_status, u16 *out_totKbSoFar);
	
	void		notify_CPU_VMCDATAFILE_TIMESTAMP (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUVMCDataFileTimeStamp &ts);
	void		translateNotify_CPU_VMCDATAFILE_TIMESTAMP (const rhea::thread::sMsg &msg, sCPUVMCDataFileTimeStamp *out);

	void		notify_WRITE_CPUFW_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, enum eWriteCPUFWFileStatus status, u16 param);
	void		translateNotify_WRITE_CPUFW_PROGRESS(const rhea::thread::sMsg &msg, eWriteCPUFWFileStatus *out_status, u16 *out_param);

	void		notify_SAN_WASHING_STATUS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 b0, u8 b1, u8 b2);
	void		translateNotify_SAN_WASHING_STATUS (const rhea::thread::sMsg &msg, u8 *out_b0, u8 *out_b1, u8 *out_b2);

	void		notify_WRITE_PARTIAL_VMCDATAFILE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 blockNumOffset);
	void		translateNotify_WRITE_PARTIAL_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_blockNumOffset);

	void		notify_CPU_DECOUNTER_SET(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_decounter which, u16 valore);
	void		translateNotify_CPU_DECOUNTER_SET(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_decounter *out_which, u16 *out_valore);

	void		notify_CPU_ALL_DECOUNTER_VALUES (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *arrayDiAlmeno13Elementi);
	void		translateNotify_CPU_ALL_DECOUNTER_VALUES(const rhea::thread::sMsg &msg, u16 *out_arrayDiAlmeno13Elementi);

	void		notify_EXTENDED_CONFIG_INFO (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sExtendedCPUInfo *info);
	void		translateNotify_EXTENDED_CONFIG_INFO(const rhea::thread::sMsg &msg, sExtendedCPUInfo *out_info);

	void		notify_ATTIVAZIONE_MOTORE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec);
	void		translateNotify_ATTIVAZIONE_MOTORE(const rhea::thread::sMsg &msg, u8 *out_motore_1_10, u8 *out_durata_dSec, u8 *out_numRipetizioni, u8 *out_pausaTraRipetizioni_dSec);

	void		notify_CALCOLA_IMPULSI_GRUPPO_STARTED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		notify_STATO_CALCOLO_IMPULSI_GRUPPO (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 stato, u16 valore);
	void		translateNotify_STATO_CALCOLO_IMPULSI_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_stato, u16 *out_valore);

	void		notify_SET_FATTORE_CALIB_MOTORE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_motor motore, u16 valore);
	void		translateNotify_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_motor *out_motore, u16 *out_valore);

	void		notify_STATO_GRUPPO (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_statoGruppo stato);
	void		translateNotify_STATO_GRUPPO(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_statoGruppo *out);

	void		notify_GET_TIME (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm, u8 ss);
	void		translateNotify_GET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss);

	void		notify_GET_DATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 year, u8 month, u8 day);
	void		translateNotify_GET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day);

	void		notify_SET_TIME(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm, u8 ss);
	void		translateNotify_SET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss);

	void		notify_SET_DATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 year, u8 month, u8 day);
	void		translateNotify_SET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day);

	void		notify_CPU_POSIZIONE_MACINA(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 macina_1o2, u16 posizione);
	void		translateNotify_CPU_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_posizione);

	void		notify_CPU_MOTORE_MACINA (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m);
	void		translateNotify_CPU_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, eCPUProgrammingCommand_macinaMove *out_m);
	
	void		notify_CPU_TEST_SELECTION(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d);
	void		translateNotify_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProgrammingCommand_testSelectionDevice *out_d);

	void		notify_NOMI_LINGE_CPU(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *strLingua1UTF16, const u16 *strLingua2UTF16);
	void		translateNotify_NOMI_LINGE_CPU(const rhea::thread::sMsg &msg, u16 *out_strLingua1UTF16, u16 *out_strLingua2UTF16);
					/*
						strLingua1UTF16 � codificato in UTF16 e termina con uno 0x0000
						out_strLingua1UTF16 deve poter accorgliere almeno 32 caratteri UTF16 pi� il terminatore 0x0000
					*/
	
	void		notify_EVA_RESET_PARTIALDATA (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool result);
	void		translateNotify_EVA_RESET_PARTIALDATA(const rhea::thread::sMsg &msg, bool *out_result);

	void		notify_GET_VOLT_AND_TEMP(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 tCamera, u8 tBollitore, u8 tCappuccinatore, u16 voltaggio);
	void		translateNotify_GET_VOLT_AND_TEMP(const rhea::thread::sMsg &msg, u8 *out_tCamera, u8 *out_tBollitore, u8 *out_tCappuccinatore, u16 *out_voltaggio);

	void		notify_GET_OFF_REPORT (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 indexNum, u8 lastIndexNum, const sCPUOffSingleEvent *offs, u8 numOffs);
	void		translateNotify_GET_OFF_REPORT(const rhea::thread::sMsg &msg, u8 *out_indexNum, u8 *out_lastIndexNum, u8 *out_numOffs, sCPUOffSingleEvent *out, u32 sizeofOut);

	void		notify_GET_LAST_FLUX_INFORMATION (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 lastFlux, u16 lastGrinderPosition);
	void		translateNotify_GET_LAST_FLUX_INFORMATION(const rhea::thread::sMsg &msg, u16 *out_lastFlux, u16 *out_lastGrinderPosition);

	void		notify_CPU_STRING_VERSION_AND_MODEL(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *utf16_msg);
	void		translateNotify_CPU_STRING_VERSION_AND_MODEL(const rhea::thread::sMsg &msg, u16 *out_utf16msg, u32 sizeOfOutUTF16MsgInBytes);

	void		notify_CPU_START_MODEM_TEST(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		notify_CPU_EVA_RESET_TOTALS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm);
	void		translateNotify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm);
	
	void		notify_START_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		notify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 fase, u8 esito, const u16 *results);
	void		translateNotify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_fase, u8 *out_esito, u16 *out_12results);

	void		notify_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 fase, u8 esito, u16 reportUP, u16 reportDOWN);
	void		translateNotify_GET_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(const rhea::thread::sMsg &msg, u8 *out_fase, u8 *out_esito, u16 *out_reportUP, u16 *out_reportDOWN);

	
	/***********************************************
		ask_xxxx
			Un subsriber di CPUBridge pu� richiedere le seguenti cose
	*/
	void		ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber);
				//alla ricezione di questo msg, CPUBridge inzier� a inviare uno o pi� notify_CPU_RUNNING_SEL_STATUS() ogni decimo di secondo circa.
				//L'ultimo notify_CPU_RUNNING_SEL_STATUS() conterr� un result "eRunningSelStatus_finished_OK" oppure "eRunningSelStatus_finished_KO" ad indicare che la selezione
				//� terminata, nel bene o nel male
	void		translate_CPU_START_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNumber);

	void		ask_CPU_STOP_SELECTION(const sSubscriber &from);
				//alla ricezione di questo msg, CPUBridge NON risponder� alcunch�
	
    void		ask_CPU_SEND_BUTTON(const sSubscriber &from, u8 buttonNum);
                //alla ricezione di questo msg, CPUBridge NON risponder� alcunch�
    void		translate_CPU_SEND_BUTTON(const rhea::thread::sMsg &msg, u8 *out_buttonNum);

    void		ask_CPU_KEEP_SENDING_BUTTON_NUM(const sSubscriber &from, u8 buttonNum);
                //alla ricezione di questo msg, CPUBridge NON risponder� alcunch�
    void		translate_CPU_KEEP_SENDING_BUTTON_NUM(const rhea::thread::sMsg &msg, u8 *out_buttonNum);

    void		ask_CPU_QUERY_RUNNING_SEL_STATUS(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con notify_CPU_RUNNING_SEL_STATUS

	void		ask_CPU_QUERY_FULLSTATE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_FULLSTATE

	void		ask_CPU_QUERY_INI_PARAM (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_INI_PARAM

	void		ask_CPU_QUERY_SEL_AVAIL(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_SEL_AVAIL_CHANGED

	void		ask_CPU_QUERY_SEL_PRICES (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_SEL_PRICES_CHANGED

	void		ask_CPU_QUERY_LCD_MESSAGE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_NEW_LCD_MESSAGE

	void		ask_CPU_QUERY_CURRENT_CREDIT(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_CREDIT_CHANGED

    void		ask_CPU_QUERY_STATE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_STATE_CHANGED

    void        ask_READ_DATA_AUDIT (const sSubscriber &from, u16 handlerID);
                    //alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_READ_DATA_AUDIT_PROGRESS.
	
	void        ask_READ_VMCDATAFILE(const sSubscriber &from, u16 handlerID);
					/* alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_READ_VMCDATAFILE_PROGRESS.
						Il da3 viene letto direttamente dalla CPU e salvato localmente nella cartella temp.
						Vedi CPUBRidgeServer::priv_downloadVMCDataFile() per ulteriori info */

	void        ask_WRITE_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
					//alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_WRITE_VMCDATAFILE_PROGRESS
	void		translate_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);


	void        ask_CPU_VMCDATAFILE_TIMESTAMP(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_VMCDATAFILE_TIMESTAMP

    void        ask_WRITE_CPUFW (const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
                     //alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_WRITE_CPUFW_PROGRESS
    void		translate_WRITE_CPUFW(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);

    void        ask_CPU_PROGRAMMING_CMD (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand cmd, const u8 *optionalData, u32 sizeOfOptionalData);
                    //invia un comando di tipo 'P' alla CPU
                    //alla ricezione di quest msg, CPUBridge non notificher� alcunche
    void		translate_CPU_PROGRAMMING_CMD(const rhea::thread::sMsg &msg, eCPUProgrammingCommand *out, const u8 **out_optionalData);

	inline void ask_CPU_PROGRAMMING_CMD_CLEANING (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_cleaningType what)					{ u8 optionalData = (u8)what; ask_CPU_PROGRAMMING_CMD(from, handlerID, eCPUProgrammingCommand_cleaning, &optionalData, 1); }
					//alla ricezione di quest msg, CPUBridge non notificher� alcunche

	inline void ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS (const sSubscriber &from, u16 handlerID)												{ ask_CPU_PROGRAMMING_CMD(from, handlerID, eCPUProgrammingCommand_querySanWashingStatus, NULL, 0); }
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_SAN_WASHING_STATUS


	void        ask_WRITE_PARTIAL_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const u8 *buffer64byte, u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_WRITE_PARTIAL_VMCDATAFILE
					//Per la spiegazione dei parametri, vedi cpubridge::buildMsg_writePartialVMCDataFile
	void		translate_PARTIAL_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_buffer64byte, u8 *out_blocco_n_di, u8 *out_tot_num_blocchi, u8 *out_blockNumOffset);

	void		ask_CPU_SET_DECOUNTER (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_decounter which, u16 valore);
	void		translate_CPU_SET_DECOUNTER(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_decounter *out_which, u16 *out_valore);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_DECOUNTER_SET

	void		ask_CPU_GET_ALL_DECOUNTER_VALUES (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_ALL_DECOUNTER_VALUES

	void		ask_CPU_GET_EXTENDED_CONFIG_INFO (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_EXTENDED_CONFIG_INFO

	void		ask_CPU_ATTIVAZIONE_MOTORE(const sSubscriber &from, u16 handlerID, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec);
	void		translate_CPU_ATTIVAZIONE_MOTORE(const rhea::thread::sMsg &msg, u8 *out_motore_1_10, u8 *out_durata_dSec, u8 *out_numRipetizioni, u8 *out_pausaTraRipetizioni_dSec);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_ATTIVAZIONE_MOTORE
	
	void		ask_CPU_CALCOLA_IMPULSI_GRUPPO (const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 totalePesata_dGrammi);
	void		translate_CPU_CALCOLA_IMPULSI_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_totalePesata_dGrammi);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CALCOLA_IMPULSI_GRUPPO_STARTED

	void		ask_CPU_GET_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_STATO_CALCOLO_IMPULSI_GRUPPO

	void		ask_CPU_SET_FATTORE_CALIB_MOTORE (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_motor motore, u16 valoreGr);
	void		translate_CPU_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_motor *out_motore, u16 *out_valoreGr);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_SET_FATTORE_CALIB_MOTORE

	void		ask_CPU_GET_STATO_GRUPPO(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_STATO_GRUPPO

	void		ask_CPU_GET_TIME(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_TIME

	void		ask_CPU_GET_DATE(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_DATE

	void		ask_CPU_SET_TIME(const sSubscriber &from, u16 handlerID, u8 hh, u8 mm, u8 ss);
	void		translate_CPU_SET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_SET_TIME

	void		ask_CPU_SET_DATE(const sSubscriber &from, u16 handlerID, u16 year, u8 month, u8 day);
	void		translate_CPU_SET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_SET_DATE

	void		ask_CPU_GET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2);
	void		translate_CPU_GET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_POSIZIONE_MACINA

	void		ask_CPU_SET_MOTORE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m);
	void		translate_CPU_SET_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, eCPUProgrammingCommand_macinaMove *out_m);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_MOTORE_MACINA

	void		ask_CPU_SET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 target);
	void		translate_CPU_SET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_target);
					//alla ricezione di questo msg, CPUBridge non notificher� alcunch�. Lo stato di CPUBridge dovrebbe passare a eVMCState_REG_APERTURA_MACINA

	void		ask_CPU_TEST_SELECTION(const sSubscriber &from, u16 handlerID, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d);
	void		translate_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProgrammingCommand_testSelectionDevice *out_d);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_TEST_SELECTION

	void		ask_CPU_GET_NOMI_LINGE_CPU(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_NOMI_LINGE_CPU

	void		ask_CPU_DISINTALLAZIONE(const sSubscriber &from);
					//alla ricezione di questo msg, CPUBridge non risponder�. Lo stato di CPU dovrebbe passare in breve tempo a eVMCState_DISINSTALLAZIONE.
					//Al termine della procedura, lo stato CPU diventa eVMCState_FINE_DISINSTALLAZIONE

	void		ask_CPU_RICARICA_FASCIA_ORARIA_FREEVEND(const sSubscriber &from);
					//alla ricezione di questo msg, CPUBridge non risponder�. La CPU dovrebbe controllare i dati nel DA3 e reimpostare la fine dell'orario di freevend

	void		ask_CPU_EVA_RESET_PARTIALDATA(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_EVA_RESET_PARTIALDATA

	void		ask_CPU_GET_VOLT_AND_TEMP(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_VOLT_AND_TEMP

	void		ask_CPU_GET_OFF_REPORT(const sSubscriber &from, u16 handlerID, u8 indexNum);
	void		translate_CPU_GET_OFF_REPORT(const rhea::thread::sMsg &msg, u8 *out_indexNum);
				//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_OFF_REPORT

	void		ask_CPU_GET_LAST_FLUX_INFORMATION(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_LAST_FLUX_INFORMATION

	void		ask_CPU_SHOW_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge non notificher� alcunch�. Il risultato per� � che per i prossimi 7 secondi, il messaggio
					//che la CPU invier� ai suoi client sar� il modello e la versione

	void		ask_CPU_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_STRING_VERSION_AND_MODEL

	void		ask_CPU_DA3SYNC(const sSubscriber &from);
					//Si richiede la sincronizzazione del file DA3 tra CPU e SMU
					//Alla ricezione di questo messaggio, lo stato di SMU passer� in eVMCState_DA3_SYNC e ci rimarr� fino alla fine
					//delle operazioni. Quando lo stato diventa != da DA3_SYNC, siamo sicuro che il da3 locale � aggiornato a quello della CPU

	void		ask_CPU_START_MODEM_TEST(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_START_MODEM_TEST

	void		ask_CPU_EVA_RESET_TOTALS(const sSubscriber &from, u16 handlerID);
				//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_EVA_RESET_TOTALS

	void		ask_CPU_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE (const sSubscriber &from, u16 handlerID);
				//alla ricezione di questo msg, CPUBridge risponder� con un notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE

	void		ask_CPU_START_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_START_TEST_ASSORBIMENTO_GRUPPO

	void		ask_CPU_START_TEST_ASSORBIMENTO_MOTORIDUTTORE (const sSubscriber &from, u16 handlerID);
				//alla ricezione di questo msg, CPUBridge risponder� con un notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE

	void		ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_GRUPPO (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_STATUS_TEST_ASSORBIMENTO_GRUPPO

	void		ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &from, u16 handlerID);
	//alla ricezione di questo msg, CPUBridge risponder� con un notify_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE

} // namespace cpubridge

#endif // _CPUBridge_h_
