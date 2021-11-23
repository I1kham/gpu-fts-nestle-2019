#ifndef _ESAPIFirmwareUpdate_h_
#define _ESAPIFirmwareUpdate_h_
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "ESAPIEnumAndDefine.h"
#include "ESAPIShared.h"

namespace esapi
{
	enum class eFirmwareUpdateType : u8
	{
		firmwareCPU = 0,
		firmwareGPU = 1,
		None = 255
	};

	// implementa la classe per la gestione del update comandato via protocollo
	class FirmwareUpdate
	{
	public:
		FirmwareUpdate();				// costruttore
		~FirmwareUpdate();				// distruttore
		bool Reset();
		bool Open(u8 type, u32 maxLen);	// apertura del file
		bool Close();					// chiusura ed applicazione delle verifiche
		bool Append(u16 block, u16 bufferLen, u8* buffer);		// aggiunge un blocco
		u16 BlockMaxDim() { return blockLen; };
		bool Complete(u16 bloackNr);

	private:
		FILE					*handle;
		eFirmwareUpdateType		type;		// tipo di file
		u16						blockLen;	// lunghezza del blocco
		u16						blockCur;	// blocco corrente;
		u32						fileLenMax;	// lunghezza del file massima
		u32						fileLenCur;	// lunghezza corrente
		u8						*fileName;	// nome del file in corso

		bool CheckUpdateCPU();
		bool CheckUpdtaeGPU();
	};
} // namespace esapi

#endif // _ESAPIFirmwareUpdate_h_
