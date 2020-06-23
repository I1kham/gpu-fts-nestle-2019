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

} // namespace esapi

#endif // _ESAPI_h_
