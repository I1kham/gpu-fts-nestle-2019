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
						[out_hThread]				è l'handle del thread che è stato creato
						[out_hServiceChannelW]		è il canale di comunicazione di "servizio" da utilizzarsi per richieste speciali (tipo subsribe)
				*/

	void		subscribe (const HThreadMsgW &hServiceChannelW, const HThreadMsgW &hAnswerHere);
					/*	Qualcuno vuole iscriversi alla coda di messaggi di output di CPUBridge.
						CPUBridge invierà la risposta a questa richiesta sul canale identificato da [hAnswerHere].

						Il thread richiedente deve quindi monitorare la propria msgQ in attesa di un msg di tipo CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER e tradurlo con
						translate_SUBSCRIPTION_ANSWER() la quale filla la struttura sCPUSubscriber da usare poi per le comunicazioni e il monitoring dei messaggi
					*/
	void		translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, sSubscriber *out, u8 *out_cpuBridgeVersion);

	void		unsubscribe (const sSubscriber &sub);

    void        loadVMCDataFileTimeStamp (sCPUVMCDataFileTimeStamp *out);
    bool        saveVMCDataFileTimeStamp(const sCPUVMCDataFileTimeStamp &ts);
	
	/***********************************************
		buildMsg_xxxx
			ritornano 0 se out_buffer non è abbastanza grande da contenere il messaggio.
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



	/***********************************************
		notify_xxxx
			il thread CPUBridge pusha questi messaggi sulle coda di uscita dei thread che si sono "subscribed" ogni volta che qualche evento importante accade oppure in risposta
			a specifiche richieste.
	*/
	void		notify_CPUBRIDGE_DYING (const sSubscriber &to);
					/*	notifica inviata a tutti i subscriber quando CPUBridge sta per morire. Il free della struttura sSubscriber è compito di CPUBridge; il subscriber deve semplicemente
						prendere atto del fatto che CPUBridge è morto ed agire di conseguenza
					*/

	void		notify_CPU_STATE_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType);
	void		translateNotify_CPU_STATE_CHANGED (const rhea::thread::sMsg &msg, eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType);

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
					/* fileID è un numero che viene appeso al nome del file durante lo scaricamento.
						Posto che il download vada a buon fine, il file localmente si trova in app/temp/dataAudit[FILE_ID].txt (es app/temp/dataAudit5.txt
					*/

	void		notify_READ_VMCDATAFILE_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID);
	void		translateNotify_READ_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID);
					/* fileID è un numero che viene appeso al nome del file durante lo scaricamento.
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
						strLingua1UTF16 è codificato in UTF16 e termina con uno 0x0000
						out_strLingua1UTF16 deve poter accorgliere almeno 16 caratteri UTF16 più il terminatore 0x0000
					*/
	

	/***********************************************
		ask_xxxx
			Un subsriber di CPUBridge può richiedere le seguenti cose
	*/
	void		ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber);
				//alla ricezione di questo msg, CPUBridge inzierà a inviare uno o più notify_CPU_RUNNING_SEL_STATUS() ogni decimo di secondo circa.
				//L'ultimo notify_CPU_RUNNING_SEL_STATUS() conterrà un result "eRunningSelStatus_finished_OK" oppure "eRunningSelStatus_finished_KO" ad indicare che la selezione
				//è terminata, nel bene o nel male
	void		translate_CPU_START_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNumber);

	void		ask_CPU_STOP_SELECTION(const sSubscriber &from);
				//alla ricezione di questo msg, CPUBridge NON risponderà alcunchè
	
    void		ask_CPU_SEND_BUTTON(const sSubscriber &from, u8 buttonNum);
                //alla ricezione di questo msg, CPUBridge NON risponderà alcunchè
    void		translate_CPU_SEND_BUTTON(const rhea::thread::sMsg &msg, u8 *out_buttonNum);

    void		ask_CPU_KEEP_SENDING_BUTTON_NUM(const sSubscriber &from, u8 buttonNum);
                //alla ricezione di questo msg, CPUBridge NON risponderà alcunchè
    void		translate_CPU_KEEP_SENDING_BUTTON_NUM(const rhea::thread::sMsg &msg, u8 *out_buttonNum);

    void		ask_CPU_QUERY_RUNNING_SEL_STATUS(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con notify_CPU_RUNNING_SEL_STATUS

	void		ask_CPU_QUERY_FULLSTATE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_FULLSTATE

	void		ask_CPU_QUERY_INI_PARAM (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_INI_PARAM

	void		ask_CPU_QUERY_SEL_AVAIL(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_SEL_AVAIL_CHANGED

	void		ask_CPU_QUERY_SEL_PRICES (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_SEL_PRICES_CHANGED

	void		ask_CPU_QUERY_LCD_MESSAGE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_NEW_LCD_MESSAGE

	void		ask_CPU_QUERY_CURRENT_CREDIT(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_CREDIT_CHANGED

    void		ask_CPU_QUERY_STATE (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_STATE_CHANGED

    void        ask_READ_DATA_AUDIT (const sSubscriber &from, u16 handlerID);
                    //alla ricezione di questo msg, CPUBridge risponderà con una o più notify_READ_DATA_AUDIT_PROGRESS.
	
	void        ask_READ_VMCDATAFILE(const sSubscriber &from, u16 handlerID);
					/* alla ricezione di questo msg, CPUBridge risponderà con una o più notify_READ_VMCDATAFILE_PROGRESS.
						Il da3 viene letto direttamente dalla CPU e salvato localmente nella cartella temp.
						Vedi CPUBRidgeServer::priv_downloadVMCDataFile() per ulteriori info */

	void        ask_WRITE_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
					//alla ricezione di questo msg, CPUBridge risponderà con una o più notify_WRITE_VMCDATAFILE_PROGRESS
	void		translate_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);


	void        ask_CPU_VMCDATAFILE_TIMESTAMP(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_VMCDATAFILE_TIMESTAMP

    void        ask_WRITE_CPUFW (const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
                     //alla ricezione di questo msg, CPUBridge risponderà con una o più notify_WRITE_CPUFW_PROGRESS
    void		translate_WRITE_CPUFW(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);

    void        ask_CPU_PROGRAMMING_CMD (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand cmd, const u8 *optionalData, u32 sizeOfOptionalData);
                    //invia un comando di tipo 'P' alla CPU
                    //alla ricezione di quest msg, CPUBridge non notificherà alcunche
    void		translate_CPU_PROGRAMMING_CMD(const rhea::thread::sMsg &msg, eCPUProgrammingCommand *out, const u8 **out_optionalData);

	inline void ask_CPU_PROGRAMMING_CMD_CLEANING (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_cleaningType what)					{ u8 optionalData = (u8)what; ask_CPU_PROGRAMMING_CMD(from, handlerID, eCPUProgrammingCommand_cleaning, &optionalData, 1); }
					//alla ricezione di quest msg, CPUBridge non notificherà alcunche

	inline void ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS (const sSubscriber &from, u16 handlerID)												{ ask_CPU_PROGRAMMING_CMD(from, handlerID, eCPUProgrammingCommand_querySanWashingStatus, NULL, 0); }
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_SAN_WASHING_STATUS


	void        ask_WRITE_PARTIAL_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const u8 *buffer64byte, u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_WRITE_PARTIAL_VMCDATAFILE
					//Per la spiegazione dei parametri, vedi cpubridge::buildMsg_writePartialVMCDataFile
	void		translate_PARTIAL_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_buffer64byte, u8 *out_blocco_n_di, u8 *out_tot_num_blocchi, u8 *out_blockNumOffset);

	void		ask_CPU_SET_DECOUNTER (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_decounter which, u16 valore);
	void		translate_CPU_SET_DECOUNTER(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_decounter *out_which, u16 *out_valore);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_DECOUNTER_SET

	void		ask_CPU_GET_ALL_DECOUNTER_VALUES (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_ALL_DECOUNTER_VALUES

	void		ask_CPU_GET_EXTENDED_CONFIG_INFO (const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_EXTENDED_CONFIG_INFO

	void		ask_CPU_ATTIVAZIONE_MOTORE(const sSubscriber &from, u16 handlerID, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec);
	void		translate_CPU_ATTIVAZIONE_MOTORE(const rhea::thread::sMsg &msg, u8 *out_motore_1_10, u8 *out_durata_dSec, u8 *out_numRipetizioni, u8 *out_pausaTraRipetizioni_dSec);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_ATTIVAZIONE_MOTORE
	
	void		ask_CPU_CALCOLA_IMPULSI_GRUPPO (const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 totalePesata_dGrammi);
	void		translate_CPU_CALCOLA_IMPULSI_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_totalePesata_dGrammi);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CALCOLA_IMPULSI_GRUPPO_STARTED

	void		ask_CPU_GET_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_STATO_CALCOLO_IMPULSI_GRUPPO

	void		ask_CPU_SET_FATTORE_CALIB_MOTORE (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_motor motore, u16 valoreGr);
	void		translate_CPU_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_motor *out_motore, u16 *out_valoreGr);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_SET_FATTORE_CALIB_MOTORE

	void		ask_CPU_GET_STATO_GRUPPO(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_STATO_GRUPPO

	void		ask_CPU_GET_TIME(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_GET_TIME

	void		ask_CPU_GET_DATE(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_GET_DATE

	void		ask_CPU_SET_TIME(const sSubscriber &from, u16 handlerID, u8 hh, u8 mm, u8 ss);
	void		translate_CPU_SET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_SET_TIME

	void		ask_CPU_SET_DATE(const sSubscriber &from, u16 handlerID, u16 year, u8 month, u8 day);
	void		translate_CPU_SET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_SET_DATE

	void		ask_CPU_GET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2);
	void		translate_CPU_GET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_POSIZIONE_MACINA

	void		ask_CPU_SET_MOTORE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m);
	void		translate_CPU_SET_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, eCPUProgrammingCommand_macinaMove *out_m);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_MOTORE_MACINA

	void		ask_CPU_SET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 target);
	void		translate_CPU_SET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_target);
					//alla ricezione di questo msg, CPUBridge non notificherà alcunchè. Lo stato di CPUBridge dovrebbe passare a eVMCState_REG_APERTURA_MACINA

	void		ask_CPU_TEST_SELECTION(const sSubscriber &from, u16 handlerID, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d);
	void		translate_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProgrammingCommand_testSelectionDevice *out_d);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_CPU_TEST_SELECTION

	void		ask_CPU_GET_NOMI_LINGE_CPU(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponderà con un notify_NOMI_LINGE_CPU

	void		ask_CPU_DISINTALLAZIONE(const sSubscriber &from);
					//alla ricezione di questo msg, CPUBridge non risponderà. Lo stato di CPU dovrebbe passare in breve tempo a eVMCState_DISINSTALLAZIONE.
					//Al termine della procedura, lo stato CPU diventa eVMCState_FINE_DISINSTALLAZIONE

} // namespace cpubridge

#endif // _CPUBridge_h_
