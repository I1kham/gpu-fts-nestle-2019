#ifndef _ESAPICore_h_
#define _ESAPICore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"
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
		static const u32		SIZE_OF_RS232BUFFEROUT = 1024;
		static const u32		SIZE_OF_SOKBUFFER = 2048;

		struct sBuffer
		{
		public:
			u8	*buffer;
			u32	numBytesInBuffer;
			u32	SIZE;

		public:
			sBuffer() { buffer = NULL; SIZE = 0; numBytesInBuffer = 0; }

			void	alloc (rhea::Allocator *allocator, u16 max_size)
					{
						this->SIZE = max_size;
						this->numBytesInBuffer = 0;
						this->buffer = (u8*)RHEAALLOC(allocator, max_size);
					}
			void	free (rhea::Allocator *allocator)
					{
						this->numBytesInBuffer = 0;
						this->SIZE = 0;
						if (NULL != this->buffer)
							RHEAFREE(allocator, this->buffer);
						this->buffer = NULL;
					}

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

		struct sConnectedSocket
		{
			u32			uid;
			OSSocket	sok;
		};

	private:
		void					priv_close();
		bool					priv_subscribeToCPUBridge();
		void					priv_handleIncomingMsgFromCPUBridge();
		void					priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg);

		void					priv_rs232_handleCommunication (OSSerialPort &comPort, sBuffer &b);
		void					priv_rs232_sendBuffer (OSSerialPort &comPort, const u8 *buffer, u32 numBytesToSend);
		bool					priv_rs232_handleCommand_A (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_C (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_R (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_S (OSSerialPort &comPort, sBuffer &b);
		
		sConnectedSocket*		priv_2280_findConnectedSocketByUID (u32 uid);
		void					priv_2280_sendDataViaRS232 (OSSocket &sok, u32 uid);
		void					priv_2280_onClientDisconnected (OSSocket &sok, u32 uid);

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
		u8						*rs232BufferOUT;
		u8						*sokBuffer;
		sRunningSel				runningSel;
		rhea::FastArray<sConnectedSocket>			sockettList;

	};


} // namespace esapi

#endif // _ESAPICore_h_
