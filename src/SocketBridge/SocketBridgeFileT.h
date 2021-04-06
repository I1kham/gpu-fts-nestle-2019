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
		static const u16		PACKET_SIZE = 900;
		static const u8			NUM_PACKET_IN_A_CHUNK = 24;

	private:
		enum class eTransferStatus: u8
		{
			pending = 0,
			finished_OK = 1,
			finished_KO = 2
		};

		struct sWhenAPPisUploading
		{
			u32 numOfPacketToBeRcvInTotal;
			u32 lastGoodPacket;
			u64 nextTimeSendNACKMsec;
		};

		struct sWhenAPPisDownloading
		{
			u8	*sendBuffer;
			u16	sizeOfSendBuffer;
			u32	numOfPacketToBeSentInTotal;
			u32 nextPacketToSend;
		};

		union sOther
		{
			sWhenAPPisUploading whenAPPisUploading;
			sWhenAPPisDownloading whenAPPisDownloading;
		};

		struct sActiveUploadRequest
		{
			eTransferStatus status;
			u8				isAppUploading;
			u32				smuFileTransfUID;
			u32				appFileTransfUID;
			u64				timeoutMSec;
			u64				nextTimeOutputANotificationMSec;
			FILE			*f;
			u32				totalFileSizeInBytes;
			u32				packetSize;
			u8				numPacketInAChunk;
			sOther			other;
		};

	private:
		void					priv_on0x01 (Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_on0x03(Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_on0x51(Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_on0x54(Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec);
		void					priv_send (Server *server, const HSokServerClient &h, const u8 *what, u16 sizeofwhat);
		u32						priv_generateUID() const;
		u32						priv_findActiveTransferBySMUUID(u32 smuFileTransfUID) const;
		void					priv_freeResources(u32 i);
		void					priv_sendChunkOfPackets(Server *server, const HSokServerClient &h, sActiveUploadRequest *s, u16 nPacket);

	private:
		rhea::Allocator			*localAllocator;
		rhea::ISimpleLogger		*logger;
		u8						*bufferW;
		rhea::FastArray<sActiveUploadRequest> activeTransferList;
		
    };

} // namespace socketbridge

#endif // _SocketBridgeFileT_h_
