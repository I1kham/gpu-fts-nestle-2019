#ifndef _rasPIMITMCore_h_
#define _rasPIMITMCore_h_
#include "rasPIMITMCoreEnumAndDefine.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"


namespace rasPI
{
	namespace MITM
	{
		class Core
		{
		public:
								Core();
								~Core() { }

			void				useLogger (rhea::ISimpleLogger *loggerIN);
			bool				open (const char *serialPortGPU, const char *serialPortCPU);
			void				run();

			HThreadMsgW			getMsgQW()						const { return msqQW; }

		private:
			static const u32	TIMEOUT_CPU_ANSWER_MSec = 5000;
			static const u32	SIZE_OF_BUFFER_GPU = 1024;
			static const u32	WAITLIST_EVENT_FROM_THREAD_MSGQ = 0x00000008;

		private:
			struct sBuffer
			{
			public:
				u8	*buffer;
				u32	numBytesInBuffer;
				u32	SIZE;

			public:
				bool	appendU8 (u8 d)
						{
							if (this->numBytesInBuffer + 1 > SIZE)
							{
								DBGBREAK;
								return false;
							}
							buffer[numBytesInBuffer++] = d;
							return true;
						}
				bool	appendU16 (u16 d)
						{
							if (this->numBytesInBuffer + 2 > SIZE)
							{
								DBGBREAK;
								return false;
							}
							buffer[numBytesInBuffer++] = (u8)((d & 0xff00) >> 8);
							buffer[numBytesInBuffer++] = (u8)(d & 0x00ff);
							return true;
						}
				bool	append (const void *src, u32 numBytesToAppend)
						{
							if (this->numBytesInBuffer + numBytesToAppend > SIZE)
							{
								DBGBREAK;
								return false;
							}
							memcpy (&buffer[numBytesInBuffer], src, numBytesToAppend);
							numBytesInBuffer += numBytesToAppend;
							return true;
						}
				void	removeFirstNBytes (u32 n)
						{
							if (n > this->numBytesInBuffer)
							{
								DBGBREAK;
								n = this->numBytesInBuffer;
							}
							const u32 nBytesLeft = this->numBytesInBuffer - n;
							if (nBytesLeft)
								memcpy (this->buffer, &this->buffer[n], nBytesLeft);
							this->numBytesInBuffer = nBytesLeft;				
						}
			};

		private:
			void				priv_close ();
			void				priv_allocBuffer(sBuffer *out, u16 max_size);
			void				priv_freeBuffer (sBuffer &b);
			bool				priv_handleMsg_send (OSSerialPort &comPort, const u8 *buffer, u16 nBytesToSend);
			bool				priv_handleMsg_rcv (OSSerialPort &comPort, u8 *out_answer, u16 *in_out_sizeOfAnswer, u64 timeoutRCVMsec);
			void				priv_handleIncomingGPUMsg (OSSerialPort &comPort, sBuffer &b);
			u32					priv_extractMessage (sBuffer &b, u8 *out, u32 sizeOfOut);
			u32					priv_isAValidMessage (const u8 *p, u32 nBytesToCheck) const;
			void				priv_handleIncomingMsgFromThreadQ();

		private:
			rhea::Allocator         *localAllocator;
			rhea::ISimpleLogger     *logger;
			rhea::NullLogger        nullLogger;
			OSSerialPort			comGPU;
			OSSerialPort			comCPU;
			char					comGPUName[32];
			char					comCPUName[32];
			bool					bQuit;
			OSWaitableGrp           waitableGrp;
			HThreadMsgR				msqQR;
			HThreadMsgW				msqQW;
			sBuffer					bufferGPU;
			sBuffer					bufferSpontaneousMsgForGPU;
			
		};
	} //namespace MITM
} // namespace rasPI
#endif // _rasPIMITMCore_h_
