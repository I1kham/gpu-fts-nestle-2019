#ifndef _ESAPICore_h_
#define _ESAPICore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../CPUBridge/CPUBridge.h"


namespace esapi
{
	class Core
	{
	public:
								Core();
								~Core()													{ }

		void					useLogger (rhea::ISimpleLogger *loggerIN);
		bool					open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW);
		void					run();

	private:
		static const u32		WAITLIST_EVENT_FROM_CPUBRIDGE	= 0x00400000;
		static const u8			API_VERSION_MAJOR = 1;
		static const u8			API_VERSION_MINOR = 0;
		static const u32		SIZE_OF_ANSWER_BUFFER = 1024;

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
			void	reset()															{ this->numBytesInBuffer = 0; }
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

		struct sRunningSel
		{
			cpubridge::eRunningSelStatus	status;
		};

	private:
		void					priv_close();
		void					priv_allocBuffer(sBuffer *out, u16 max_size);
		void					priv_freeBuffer (sBuffer &b);
		bool					priv_subscribeToCPUBridge();
		void					priv_handleIncomingMsgFromCPUBridge();
		void					priv_handleSerialCommunication (OSSerialPort &comPort, sBuffer &b);
		void					priv_sendBuffer (OSSerialPort &comPort, const u8 *buffer, u32 numBytesToSend);
		void					priv_buildAndSendAnswer (OSSerialPort &comPort, u8 commandChar, const u8* optionalData, u32 numOfBytesInOptionalData);
		bool					priv_utils_parseCommand (sBuffer &b, u32 expectedCommandLen, bool *out_atLeastOneByteConsumed);
		bool					priv_handleCommand_A (OSSerialPort &comPort, sBuffer &b);
		bool					priv_handleCommand_C (OSSerialPort &comPort, sBuffer &b);
		bool					priv_handleCommand_S (OSSerialPort &comPort, sBuffer &b);
		void					priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg);

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		rhea::NullLogger        nullLogger;
		OSSerialPort			com;
		HThreadMsgW				hCPUServiceChannelW;
		OSWaitableGrp           waitableGrp;
		sBuffer					serialBuffer;
		bool					bQuit;
		cpubridge::sSubscriber	cpuBridgeSubscriber;
		u8						*answerBuffer;
		sRunningSel				runningSel;

	};


} // namespace esapi

#endif // _ESAPICore_h_
