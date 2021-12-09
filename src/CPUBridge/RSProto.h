#ifndef _RSProto_h_
#define _RSProto_h_
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "CPUBridgeEnumAndDefine.h"

namespace cpubridge
{
	class Server;
}

/******************************************************************************
* 
**/
class RSProto
{
	public:
		static const u16 FILE_EVA_DTS					= 0x0001;
		static const u16 FILE_MACHINE_CONFIG			= 0x0002;
		static const u16 FILE_SMU						= 0x0003;
		static const u16 FILE_CPU						= 0x0004;
		static const u16 FILE_GUI						= 0x0005;

public:
				RSProto (rhea::ProtocolSocketServer *serverTCP, const HSokServerClient hClient);
				~RSProto ();

	void		onMessageRCV (cpubridge::Server *server, u16 what, u16 userValue, const u8 *payload, u32 payloadLen, rhea::ISimpleLogger *logger);
	void		onSMUStateChanged (const cpubridge::sCPUStatus &status, rhea::ISimpleLogger *logger);
	void		sendTemperature (u8 temperatureEsp, u8 temperatureCamCaffe, u8 temperatureSol, u8 temperatureIce, u8 temperatureMilker, rhea::ISimpleLogger *logger);
	void		sendFileAnswer (u32 fileID, u16 userValue, const u8 *fullPathName);
	
	bool		isItMe (const HSokServerClient hClientIN) const					{ return hClientIN == hClient; }

public:
	u64			nextTimeSendTemperature_msec;

private:
	static const u8  MACHINE_TYPE_TT = 0x01;
	//static const u8  MACHINE_TYPE_FS_LUCE = 0x02;

	static const u8  VAR_TYPE_u32 = 0x01;

	static const u32 VAR_STATO_MACCHINA				= 0x00000001;
	static const u32 VAR_TEMPERATURA_ESPRESSO		= 0x00000004;
	static const u32 VAR_TEMPERATURA_SOLUBILE		= 0x00000005;
	static const u32 VAR_TEMPERATURA_CAM_CAFFE		= 0x00000006;
	static const u32 VAR_TEMPERATURA_BANCO_GHIACCIO = 0x00000007;
	static const u32 VAR_TEMPERATURA_MILKER			= 0x00000008;

private:
	enum class eAnswerToFileRequest
	{
		notAvail = 0,
		wait = 1,
		sendFile = 2
	};

private:
	void		priv_send (u16 what, u16 userValue, const void *payload, u32 payloadLen);
	void		priv_reset();
	void		priv_sendLastSMUState (rhea::ISimpleLogger *logger);
	void		priv_RSProtoSentMeAFile (cpubridge::Server *server, u32 fileID, const u8 *fileFullPath, rhea::ISimpleLogger *logger);
	void		priv_RSProtoWantsAFile (cpubridge::Server *server, u32 fileID, u16 userValue, rhea::ISimpleLogger *logger);
	bool		priv_copyFileInTempFolder (const u8 *fullSrcFilePathAndName, const char *fileExt, u8 *out_fullFilePathAndName, u32 sizeof_outFullFilePathAndName) const;

private:
	rhea::ProtocolSocketServer	*serverTCP;
	HSokServerClient			hClient;
	u8							lastStatus[3];
	u8							lastTemperature[5];
};

#endif _RSProto_h_
