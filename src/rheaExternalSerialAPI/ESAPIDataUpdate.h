#ifndef _ESAPIDataUpdate_h_
#define _ESAPIDataUpdate_h_
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "ESAPIEnumAndDefine.h"
#include "ESAPIShared.h"

namespace esapi
{
	enum class eDataUpdateType : u8
	{
		CPU = 0,
		GPU = 1,
		GUI = 2,
		DataFile = 3,
		None = 255
	};

	// implementa la classe per la gestione del update comandato via protocollo
	class DataUpdate
	{
	public:
		DataUpdate();				// costruttore
		~DataUpdate();				// distruttore
		bool Reset();
		bool Open(u8 type, u32 maxLen);	// apertura del file
		bool Append(u16 block, u16 bufferLen, u8* buffer);		// aggiunge un blocco
		u16 BlockMaxDim() { return blockLen; };
		bool Complete(u16 blockNr, const cpubridge::sSubscriber& from);

	private:
		FILE				*handle;
		eDataUpdateType		type;		// tipo di file
		u16					blockLen;	// lunghezza del blocco
		u16					blockCur;	// blocco corrente;
		u32					fileLenMax;	// lunghezza del file massima
		u32					fileLenCur;	// lunghezza corrente
		u8					*fileName;	// nome del file in corso

		bool UpdateCPU(const cpubridge::sSubscriber& from);
		bool UpdateGPU(const cpubridge::sSubscriber& from);
		bool UpdateGUI(const cpubridge::sSubscriber& from);
		bool UpdateDA3(const cpubridge::sSubscriber& from);
	};
} // namespace esapi

#endif // _ESAPIDataUpdate_h_
