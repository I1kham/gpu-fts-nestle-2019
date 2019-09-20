#ifndef _SocketBridgeFileT_h_
#define _SocketBridgeFileT_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/rheaRandom.h"
#include "SocketBridgeEnumAndDefine.h"
#include "SocketBridgeFileTEnumAndDefine.h"

namespace socketbridge
{
	class Server;

    class FileTransfer
    {
    public:
								FileTransfer();
								~FileTransfer()																												{ unsetup(); }

		void					setup (rhea::Allocator *allocator, rhea::ISimpleLogger *loggerIN);
		void					unsetup();

		void					update(u64 timeNowMSec);
		void					handleMsg (Server *server, const HSokServerClient &h, socketbridge::sDecodedMessage &decoded, u64 timeNowMSec);

	private:
		static const u16		SIZE_OF_BUFFERW = 1024 + 256;

	private:
		enum eTransferStatus
		{
			eTransferStatus_pending = 0,
			eTransferStatus_finished_OK = 1,
			eTransferStatus_finished_KO = 2
		};

		struct sActiveUploadRequest
		{
			eTransferStatus status;
			u32	smuFileTransfUID;
			u32 appFileTransfUID;
			u32 totalFileSizeInBytes;
			u32 packetSize;
			u32 numOfPacketToBeRcvInTotal;
			u64	timeoutMSec;
			u64 nextTimeOutputANotificationMSec;
			u32 lastGoodPacket;
			FILE *f;
		};

	private:
		void					priv_on_Opcode_upload_request_fromApp (Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_on_Opcode_upload_request_fromApp_packet(Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_send (Server *server, const HSokServerClient &h, const u8 *what, u16 sizeofwhat);
		u32						priv_generateUID() const;
		u32						priv_findActiveTransferBySMUUID(u32 smuFileTransfUID) const;
		void					priv_freeResources(u32 i);

	private:
		rhea::Allocator			*localAllocator;
		rhea::ISimpleLogger		*logger;
		u8						*bufferW;
		rhea::FastArray<sActiveUploadRequest> activeTransferList;
		
    };

} // namespace socketbridge

#endif // _SocketBridgeFileT_h_
