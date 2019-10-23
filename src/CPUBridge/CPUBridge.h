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
	u8			buildMsg_writeVMCDataFile(const u8 *buffer64yteLettiDalFile, u8 blockNum, u8 totNumBlocks, u8 *out_buffer, u8 sizeOfOutBuffer);
	u8			buildMsg_getVMCDataFileTimeStamp (u8 *out_buffer, u8 sizeOfOutBuffer);



	/***********************************************
		notify_xxxx
			il thread CPUBridge pusha questi messaggi sulle coda di uscita dei thread che si sono "subscribed" ogni volta che qualche evento importante accade oppure in risposta
			a specifiche richieste.
	*/
	void		notify_CPUBRIDGE_DYING (const sSubscriber &to);
					/*	notifica inviata a tutti i subscriber quando CPUBridge sta per morire. Il free della struttura sSubscriber � compito di CPUBridge; il subscriber deve semplicemente
						prendere atto del fatto che CPUBridge � morto ed agire di conseguenza
					*/

	void		notify_CPU_STATE_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType);
	void		translateNotify_CPU_STATE_CHANGED (const rhea::thread::sMsg &msg, eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType);

	void		notify_CPU_NEW_LCD_MESSAGE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPULCDMessage *msg);
	void		translateNotify_CPU_NEW_LCD_MESSAGE(const rhea::thread::sMsg &msg, sCPULCDMessage *out_msg);

	void		notify_CPU_CREDIT_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const void *credit, u8 sizeOfCredit);
	void		translateNotify_CPU_CREDIT_CHANGED (const rhea::thread::sMsg &msg, u8 *out_credit, u16 sizeOfOut);
	
	void		notify_CPU_SEL_AVAIL_CHANGED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUSelAvailability *s);
	void		translateNotify_CPU_SEL_AVAIL_CHANGED (const rhea::thread::sMsg &msg, sCPUSelAvailability *out);

	void		notify_CPU_SEL_PRICES_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *prices, u32 sizeOfPricesArray);
	void		translateNotify_CPU_SEL_PRICES_CHANGED(const rhea::thread::sMsg &msg, u16 *out_prices, u32 sizeOfPricesArray);

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

	void		notify_WRITE_CPUFW_PROGRESS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eWriteCPUFWFileStatus status, u16 param);
	void		translateNotify_WRITE_CPUFW_PROGRESS(const rhea::thread::sMsg &msg, eWriteCPUFWFileStatus *out_status, u16 *out_param);

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
				//alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_READ_VMCDATAFILE_PROGRESS.

	void        ask_WRITE_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
					//alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_WRITE_VMCDATAFILE_PROGRESS
	void		translate_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);


	void        ask_CPU_VMCDATAFILE_TIMESTAMP(const sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, CPUBridge risponder� con un notify_CPU_VMCDATAFILE_TIMESTAMP

    void        ask_WRITE_CPUFW (const sSubscriber &from, u16 handlerID, const char *srcFullFileNameAndPath);
                     //alla ricezione di questo msg, CPUBridge risponder� con una o pi� notify_WRITE_CPUFW_PROGRESS
    void		translate_WRITE_CPUFW(const rhea::thread::sMsg &msg, char *out_srcFullFileNameAndPath, u32 sizeOfOut);

} // namespace cpubridge

#endif // _CPUBridge_h_
