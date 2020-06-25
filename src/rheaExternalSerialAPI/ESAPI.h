#ifndef _ESAPI_h_
#define _ESAPI_h_
#include "ESAPIEnumAndDefine.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"


namespace esapi
{
	bool        startThread (const char *comPort, const HThreadMsgW &hCPUServiceChannelW, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread);
				/*
					Ritorna false in caso di problemi.
					Se ritorna true, allora:
						[out_hThread]				è l'handle del thread che è stato creato
				*/

	void		subscribe (const HThreadMsgW &hAnswerHere);
					/*	Qualcuno vuole iscriversi alla coda di messaggi di output del thread di ESAPI.
						ESAPI invierà la risposta a questa richiesta sul canale identificato da [hAnswerHere].

						Il thread richiedente deve quindi monitorare la propria msgQ in attesa di un msg di tipo ESAPI_SERVICECH_SUBSCRIPTION_ANSWER e tradurlo con
						translate_SUBSCRIPTION_ANSWER() la quale filla la struttura sSubscriber da usare poi per le comunicazioni e il monitoring dei messaggi
					*/
	void		translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, cpubridge::sSubscriber *out);

	void		unsubscribe (const cpubridge::sSubscriber &sub);


	bool		isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse);



	/***********************************************
		buildMsg_xxxx
			ritornano 0 se out_buffer non è abbastanza grande da contenere il messaggio.
			altrimenti ritornano il num di byte inseriti in out_buffer

		buildMsg_xxxx_parse..
			ritornano 0 se nel buffer non ci sono abbastanza bytes da completare l'intera risposta.
			Se invece la risposta c'è tutta, ritornano il num di bytes utillizzati e valorizzano [out_bIsValidCk]==true se la ck è valida, false altrimenti
	*/
	u32			buildMsg_A1_getAPIVersion_ask (u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_A1_getAPIVersion_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk);
	u32			buildMsg_A1_getAPIVersion_resp (u8 apiVerMajor, u8 apiVerMinor, eGPUType gpuType, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_A1_getAPIVersion_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_apiVerMajor, u8 *out_apiVerMinor, eGPUType *out_gpuType);

	u32			buildMsg_C1_getCPUScreenMsg_ask (u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_C1_getCPUScreenMsg_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk);
	u32			buildMsg_C1_getCPUScreenMsg_resp (const void *lcdMsg, u16 msgLenInBytes, u8 *out_buffer, u32 sizeOfOutBuffer);

	u32			buildMsg_C2_getSelAvailability_ask (u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_C2_getSelAvailability_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk);
	u32			buildMsg_C2_getSelAvailability_resp (const cpubridge::sCPUSelAvailability &selAvail, u8 *out_buffer, u32 sizeOfOutBuffer);


	u32			buildMsg_S1_startSelection_ask (u8 selNum, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_S1_startSelection_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_selNum);
	u32			buildMsg_S1_startSelection_resp (u8 selNum, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_S1_startSelection_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_selNum);

	u32			buildMsg_S2_querySelectionStatus_ask (u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_S2_querySelectionStatus_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk);
	u32			buildMsg_S2_querySelectionStatus_resp (cpubridge::eRunningSelStatus status, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_S2_querySelectionStatus_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, cpubridge::eRunningSelStatus *out_sstatus);


	u32			buildMsg_R1_externalModuleIdentify_ask (eExternalModuleType moduleType, u8 verMajor, u8 verMinor, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_R1_externalModuleIdentify_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, eExternalModuleType *out_moduleType, u8 *out_verMajor, u8 *out_verMinor);
	u32			buildMsg_R1_externalModuleIdentify_resp (u8 result, u8 *out_buffer, u32 sizeOfOutBuffer);
	
	u32			buildMsg_R0x01_newSocket (u32 socketUID, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_R0x01_newSocket_parse (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u32 *out_socketUID);
	
	u32			buildMsg_R0x02_closeSocket (u32 socketUID, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_R0x02_closeSocket_parse (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u32 *out_socketUID);
	
	u32			buildMsg_R0x03_socketDataToGPU (u32 socketUID, const u8 *data, u16 lenOfData, u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_R0x04_GPUDataToSocket (u32 socketUID, const u8 *data, u16 lenOfData, u8 *out_buffer, u32 sizeOfOutBuffer);

	u32			buildMsg_R0x05_getIPandSSID (u8 *out_buffer, u32 sizeOfOutBuffer);
	u32			buildMsg_R0x06_start (u8 *out_buffer, u32 sizeOfOutBuffer);



	/***********************************************
		ask_xxxx
			Un subscriber di ESAPI può richiedere le seguenti cose
	*/

	void		ask_UNSUBSCRIBE (const cpubridge::sSubscriber &from);

	void		ask_GET_MODULE_TYPE_AND_VER (const cpubridge::sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, ESAPI risponderà con notify_MODULE_TYPE_AND_VER

	void		notify_MODULE_TYPE_AND_VER(const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eExternalModuleType type, u8 verMajor, u8 verMinor);
	void		translateNotify_MODULE_TYPE_AND_VER(const rhea::thread::sMsg &msg, eExternalModuleType *out_type, u8 *out_verMajor, u8 *out_verMinor);


	/***********************************************
		Posto che a seguito della notify_MODULE_TYPE_AND_VER() si sia capito che stiamo parlando con un modulo eExternalModuleType_rasPI_wifi_REST,
		allora i seguenti sono validi
		ask_xxxx
			Un subscriber di ESAPI può richiedere le seguenti cose
	*/
	void		ask_RASPI_GET_WIFI_IPandSSID (const cpubridge::sSubscriber &from, u16 handlerID);
                    //alla ricezione di questo msg, ESAPI risponderà con notify_RASPI_WIFI_IPandSSID
	void		notify_RASPI_WIFI_IPandSSID (const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 ipPart0, u8 ipPart1, u8 ipPart2, u8 ipPart3, const char *ssid);
    void		translateNotify_RASPI_WIFI_IPandSSID(const rhea::thread::sMsg &msg, char *out_ipAddress, u32 sizeof_outIpAddress, char *out_ssid, u32 sizeof_outssid);

	void		ask_RASPI_START (const cpubridge::sSubscriber &from, u16 handlerID);
					//alla ricezione di questo msg, ESAPI risponderà con notify_RASPI_STARTED
	void		notify_RASPI_STARTED(const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger);

	void		ask_RASPI_START_FILEUPLOAD (const cpubridge::sSubscriber &from, const u8* const fullFilePathAndName);
	void		translate_RASPI_START_FILEUPLOAD(const rhea::thread::sMsg &msg, const u8 **out_pointerToFullFilePathAndName);
					//alla ricezione di questo msg, ESAPI risponderà con una serie di notify_RASPI_FILEUPLOAD ad indicare lo stato dell'upload
	void		notify_RASPI_FILEUPLOAD(const cpubridge::sSubscriber &to, rhea::ISimpleLogger *logger, eFileUploadStatus status, u32 kbSoFar);
	void		translateNotify_RASPI_FILEUPLOAD(const rhea::thread::sMsg &msg, eFileUploadStatus *out_status, u32 *out_kbSoFar);
} // namespace esapi

#endif // _ESAPI_h_
