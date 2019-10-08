#ifndef _SocketBridgeFileTEnumAndDefine_h_
#define _SocketBridgeFileTEnumAndDefine_h_
#include "../rheaCommonLib/rheaDataTypes.h"
#include "../rheaCommonLib/rheaNetBufferView.h"

namespace socketbridge
{
	enum eFileTransferOpcode
	{
		eFileTransferOpcode_upload_request_fromApp				= 0x01,	//SMU ha ricevuto una richiesta di upload da APP
		eFileTransferOpcode_upload_request_fromApp_answ			= 0x02,	//SMU risponde a APP in relazione al msg precedente
		eFileTransferOpcode_upload_request_fromApp_packet		= 0x03,	//APP manda a SMU un chunck di dati
		eFileTransferOpcode_upload_request_fromApp_packet_answ	= 0x04, //SMU risponde a APP in relazione al msg precedente

		eFileTransferOpcode_download_request_fromApp			= 0x51,	//SMU ha ricevuto una richiesta di download da APP
		eFileTransferOpcode_download_request_fromApp_answ		= 0x52,	//SMU risponde a APP in relazione al msg precedente
		eFileTransferOpcode_download_request_fromApp_packet		= 0x53,	//SMU manda a APP un chunck di dati
		eFileTransferOpcode_download_request_fromApp_packet_answ= 0x54, //APP risponde a SMU in relazione al msg precedente

		eFileTransferOpcode_unknown = 0xff
	};

	enum eFileTransferFailReason
	{
		eFileTransferFailReason_none = 0x00,
		eFileTransferFailReason_timeout = 0x01,
		eFileTransferFailReason_smuRefused = 0x02,
		eFileTransferFailReason_localReadBufferTooShort = 0x03,
		eFileTransferFailReason_smuErrorOpeningFile = 0x04,
		eFileTransferFailReason_smuFileTooBigOrEmpty = 0x05,

		eFileTransferFailReason_unkwnown = 0xff
	};

	namespace fileT
	{
		struct sData0x01
		{
			static const u8 MAX_USAGE_LEN = 255;
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_upload_request_fromApp;

					sData0x01()						{ opcode = OPCODE; }
			u8		opcode;
			u8		usageLen;
			u16		packetSizeInBytes;
			u32		fileSizeInBytes;
			u32		appTransfUID;
			char	usage[MAX_USAGE_LEN];
			u8		numPacketInAChunk;

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU8(usageLen);
				nbw.writeU16(packetSizeInBytes);
				nbw.writeU32(fileSizeInBytes);
				nbw.writeU32(appTransfUID);
				nbw.writeU8(numPacketInAChunk);
				nbw.writeBlob(usage, usageLen);
				return (u16)nbw.length();
			}

			bool	decode (rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 13)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU8(this->usageLen);
				nbr.readU16(this->packetSizeInBytes);
				nbr.readU32(this->fileSizeInBytes);
				nbr.readU32(this->appTransfUID);
				nbr.readU8(this->numPacketInAChunk);

				if (this->usageLen > (MAX_USAGE_LEN-1))
					return false;
				if (nbr.length() < (u32)(13 + (u32)this->usageLen))
					return false;

				nbr.readBlob(this->usage, this->usageLen);
				this->usage[this->usageLen] = 0x00;
				return true;
			}
		};

		struct sData0x02
		{
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_upload_request_fromApp_answ;

					sData0x02()						{ opcode = OPCODE; }
			u8		opcode;
			u8		reason_refused;
			u16		packetSizeInBytes;
			u32		smuTransfUID;
			u32		appTransfUID;
			u8		numPacketInAChunk;

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU8(reason_refused);
				nbw.writeU16(packetSizeInBytes);
				nbw.writeU32(smuTransfUID);
				nbw.writeU32(appTransfUID);
				nbw.writeU8(numPacketInAChunk);
				return (u16)nbw.length();
			}
			bool	decode(rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 13)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU8(this->reason_refused);
				nbr.readU16(this->packetSizeInBytes);
				nbr.readU32(this->smuTransfUID);
				nbr.readU32(this->appTransfUID);
				nbr.readU8(this->numPacketInAChunk);
				return true;
			}
		};

		struct sData0x04
		{
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_upload_request_fromApp_packet_answ;

					sData0x04()				{ opcode = OPCODE; }
			u8		opcode;
			u32		appTransfUID;
			u32		packetNumAccepted;

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU32(appTransfUID);
				nbw.writeU32(packetNumAccepted);
				return (u16)nbw.length();
			}
			bool	decode(rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 9)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU32(this->appTransfUID);
				nbr.readU32(this->packetNumAccepted);
				return true;
			}
		};


		struct sData0x51
		{
			static const u8 MAX_WHAT_LEN = 32;
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_download_request_fromApp;

			sData0x51() { opcode = OPCODE; }
			u8		opcode;
			u8		whatLen;
			u32		appTransfUID;
			char	what[MAX_WHAT_LEN];

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU8(whatLen);
				nbw.writeU32(appTransfUID);
				nbw.writeBlob(what, whatLen);
				return (u16)nbw.length();
			}

			bool	decode(rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 6)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU8(this->whatLen);
				nbr.readU32(this->appTransfUID);

				if (this->whatLen > (MAX_WHAT_LEN - 1))
					return false;
				if (nbr.length() < (u32)(6 + (u32)this->whatLen))
					return false;

				nbr.readBlob(this->what, this->whatLen);
				this->what[this->whatLen] = 0x00;
				return true;
			}
		};

		struct sData0x52
		{
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_download_request_fromApp_answ;

			sData0x52()						{ opcode = OPCODE; }
			u8		opcode;
			u8		reason_refused;
			u16		packetSizeInBytes;
			u32		smuTransfUID;
			u32		appTransfUID;
			u32		fileSize;
			u8		numPacketInAChunk;

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU8(reason_refused);
				nbw.writeU16(packetSizeInBytes);
				nbw.writeU32(smuTransfUID);
				nbw.writeU32(appTransfUID);
				nbw.writeU32(fileSize);
				nbw.writeU8(numPacketInAChunk);
				return (u16)nbw.length();
			}
			bool	decode(rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 17)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU8(this->reason_refused);
				nbr.readU16(this->packetSizeInBytes);
				nbr.readU32(this->smuTransfUID);
				nbr.readU32(this->appTransfUID);
				nbr.readU32(this->fileSize);
				nbr.readU8(this->numPacketInAChunk);
				return true;
			}
		};

		struct sData0x54
		{
			static const u8 OPCODE = (u8)socketbridge::eFileTransferOpcode_download_request_fromApp_packet_answ;

			sData0x54() { opcode = OPCODE; }
			u8		opcode;
			u32		smuFileTransfUID;
			u32		packetNumAccepted;

			u16		encode(u8 *buffer, u32 sizeOfBuffer)
			{
				assert(opcode == OPCODE);

				rhea::NetStaticBufferViewW nbw;
				nbw.setup(buffer, sizeOfBuffer, rhea::eBigEndian);

				nbw.writeU8(opcode);
				nbw.writeU32(smuFileTransfUID);
				nbw.writeU32(packetNumAccepted);
				return (u16)nbw.length();
			}
			bool	decode(rhea::NetStaticBufferViewR &nbr)
			{
				nbr.seek(0);
				if (nbr.length() < 9)
					return false;

				nbr.readU8(this->opcode);
				nbr.readU32(this->smuFileTransfUID);
				nbr.readU32(this->packetNumAccepted);
				return true;
			}
		};
	} //namespace fileT
} // namespace socketbridge

#endif // _SocketBridgeFileTEnumAndDefine_h_

