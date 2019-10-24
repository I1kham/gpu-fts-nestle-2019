#ifndef _rheaAppFileTransfer_h_
#define _rheaAppFileTransfer_h_
#include "rheaAppEnumAndDefine.h"
#include "../rheaCommonLib/rhea.h"
#include "../SocketBridge/SocketBridgeEnumAndDefine.h"
#include "../rheaCommonLib/Protocol/IProtocol.h"
#include "../rheaCommonLib/Protocol/IProtocolChannell.h"
#include "../rheaCommonLib/rheaHandleUID88.h"
#include "../rheaCommonLib/rheaHandleArray.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/rheaFIFO.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"

namespace rhea
{
	namespace app
	{
		/****************************************************************
		 * FileTransfer
		 *
		 * Gestore di file upload/download verso SMU
		 */
		class FileTransfer
		{
		public:
			RHEATYPEDEF_HANDLE88(Handle);

			struct sTansferInfo
			{
				Handle				handle;
				u32					totalTransferSizeInBytes;
				u32					currentTransferSizeInBytes;
				u64					timeElapsedMSec;
				eFileTransferStatus status;
				socketbridge::eFileTransferFailReason failReason;
			};

		public:
							FileTransfer()																			{ localAllocator = NULL; bufferReadFromFile = NULL; }
							~FileTransfer()																			{ unsetup();  }

			void			setup (rhea::Allocator *allocator, rhea::ISimpleLogger *loggerIN);
			void			unsetup();


			bool			update (u64 timeNowMSec);
							/*	Da chiamare periodicamente per aggiornare lo stato dei trasferimenti in corso.
								Ritorna true se ci sono "eventi" in coda; questi eventi sono recuperatibili tramite la popEvent()
							*/

			void			onMessage(u64 timeNowMSec, const sDecodedFileTransfMsg &msg);
							//da chiamare ogni volta che viene ricevuto un msg di tipo fileTransfer
			
			bool			popEvent(sTansferInfo *out)																{ return eventList.pop(out); }
							//ritorna il primo evento in lista (FIFO) se ce ne sono

			
			bool			getTransferInfo (const Handle &h, sTansferInfo *out);
							//per ottenere info sullo stato di uno specifico transfer

			bool			startFileUpload (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u64 timeNowMSec, const char *fullFileNameAndPath, const char *usage, Handle *out);
							//ritorna true se SMU ha accettato il trasferimento. Da qui in poi, il trasferimento viene portato avanti in automatico posto che
							//la update() venga chiamata con una certa frequenza e che la onMessage() venga chiamata ogni volta che si riceve un msg
							//relativo al file transfer

			bool			startFileDownload(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u64 timeNowMSec, const char *what, const char *dstFullFileNameAndPath, Handle *out);
							//ritorna true se SMU ha accetto il download. Alla fine del procedimento, il file downloadato da SMU viene messo in [dstFullFileNameAndPath]
			
		private:
			static const u8		MAX_SIMULTANEOUS_TRANSFER		= 16;
			static const u16	SIZE_OF_READ_BUFFER_FROM_FILE	= 1024 + 32;

		private:
			struct sWhenUploading
			{
				u8								*sendBuffer;
				u16								sizeOfSendBuffer;
				u32								packetNum;
				u32								numOfPacketToBeSentInTotal;
			};

			struct sWhenDownloading
			{
				u32 lastGoodPacket;
				u32 numOfPacketToBeRcvInTotal;
				u64 nextTimeSendNACKMsec;
			};

			union sData
			{
				sWhenUploading		whenUploading;
				sWhenDownloading	whenDownloading;
			};

			struct sRecord
			{
				FILE							*f;
				rhea::IProtocolChannell			*ch;
				rhea::IProtocol					*proto;
				app::FileTransfer::Handle		handle;
				u64								timeoutMSec;
				u64								timeStartedMSec;
				u64								nextTimePushAnUpdateMSec;
				u32								smuFileTransfUID;
				u32								fileSizeInBytes;
				u16								packetSize;
				u8								numPacketInAChunk;
				u8								status;
				u8								whatAmIDoing;
				u8								isAnUpload;
				sData							other;

				void oneTimeInit() {}
				void oneTimeDeinit() {}
				void onAlloc() {}
				void onDealloc() {}

			};

		private:
			void						priv_freeResources(sRecord *s) const;
			bool						priv_isUploading(const sRecord *s) const;
			void						priv_failed(sRecord *s, socketbridge::eFileTransferFailReason failReason, bool bAppendFailEvent);
			void						priv_fillTransferInfo(const sRecord *s, sTansferInfo *out) const;
			sRecord*					priv_fromHandleAsU32(u32 hAsU32) const;
			void						priv_sendChunkOfPackets(sRecord *s, u16 nPacket);
			//void						priv_advanceAndSendPacket(sRecord *s);
			//void						priv_doSendPacket(sRecord *s, u32 iPacket, u32 fileOffset, u16 packetSize) const;
			void						priv_on0x02(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr);
			void						priv_on0x04(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr);
			void						priv_on0x52(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr);
			void						priv_on0x53(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr);

		private:
			rhea::Allocator					*localAllocator;
			rhea::ISimpleLogger				*logger;
			rhea::HandleArray<sRecord, app::FileTransfer::Handle> handleArray;
			rhea::FastArray<Handle>			activeHandleList;
			rhea::FIFO<sTansferInfo>		eventList;
			u8								*bufferReadFromFile;
		};

	} // namespace app

} // namespace rhea


#endif // _rheaAppFileTransfer_h_

