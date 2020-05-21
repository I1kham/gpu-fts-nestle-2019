#ifndef _socketListenerCore_h_
#define _socketListenerCore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaLinearBuffer.h"
#include "../SocketBridge/SocketBridgeEnumAndDefine.h"
#include "rasPIIdentifiedClientList.h"
#include "CommandHandlerList.h"

namespace rasPI
{
	namespace socketListener
	{
		class Core
		{
		public:
									Core();
									~Core()															{ }

			void					useLogger (rhea::ISimpleLogger *loggerIN);
			bool					open (u16 tcpPortNumber);
			void					run();
		
			void					sendTo (const HSokServerClient &h, const u8 *buffer, u32 nBytesToSend);
			void					sendEvent (const HSokServerClient &h, socketbridge::eEventType eventType, const void *optionalData, u16 lenOfOptionalData);

		private:
			u16						priv_getANewHandlerID ();
			bool					priv_encodeMessageOfTypeEvent (socketbridge::eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength);
			void					priv_handleIdentification (const HSokServerClient &h, const socketbridge::sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded);
			bool					priv_extractOneMessage(u8 *bufferPt, u16 nBytesInBuffer, socketbridge::sDecodedMessage *out, u16 *out_nBytesConsumed) const;
			void					priv_onClientHasDataAvail(u8 iEvent, u64 timeNowMSec);
			void					priv_onClientHasDataAvail2(u64 timeNowMSec, HSokServerClient &h, const socketbridge::sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded);

		private:
			static const u16        RESERVED_HANDLE_RANGE = 1024;

		private:
			rhea::Allocator         *localAllocator;
			rhea::ISimpleLogger     *logger;
			rhea::NullLogger        nullLogger;
			rhea::ProtocolSocketServer    *server;
			u16						SEND_BUFFER_SIZE;
			u8						*sendBuffer;
			rhea::LinearBuffer      buffer;
			socketbridge::IdentifiedClientList    identifiedClientList;
			socketbridge::CommandHandlerList      cmdHandlerList;
			u8						eventSeqNumber;
			u16                     _nextHandlerID;
		};

	} //socketListener
} //namespace rasPI

#endif //_socketListenerCore_h_