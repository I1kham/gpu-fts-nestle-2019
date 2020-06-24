#ifndef _raspiCore_h_
#define _raspiCore_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaExternalSerialAPI/ESAPI.h"

namespace raspi
{
	/*************************************************************
	 * Core
	 *
	 *	Apre una socket TCP/IP sulla porta 2280 e rimane in attesa di connessioni.
	 *	Ogni volta che una socket si connette, crea un record in [clientList] e da li in poi tutti i dati in arrivo su quella socket vengono
	 *	inviati lungo la rs232 alla GPU.
	 *	Il canale funziona anche nell'altro senso ovvero, i dati in transito lungo la rs232 provenienti dalla GPU, vengono inviati lungo la socket
	 *	L'idea � di mascherare il fatto di avere una rs232 in mezzo e far credere al client che si collega alla 2280 di stare direttamente parlando con la GPU dall'altro lato
	 *	della rs232
	 */
	class Core
	{
	public:
						Core ();
						~Core ()																{ priv_close(); }

		void            useLogger (rhea::ISimpleLogger *loggerIN);

		bool			open (const char *serialPort);
		void			run ();

	private:
		static const u8		VER_MAJOR = 1;
		static const u8		VER_MINOR = 0;
		static const u32	WAITGRP_SOCKET2280		= 0xFFFFFFFF;
		
		
		static const u16	SIZE_OF_RS232BUFFEROUT	= 8192;
		static const u16	SIZE_OF_RS232BUFFERIN	= 8192;
		static const u16	SOKCLIENT_BUFFER_SIZE	= 4096;
		static const u16	SOK_BUFFER_SIZE			= 8192;

	private:
		struct sConnectedSocket
		{
			u32			uid;
			OSSocket	sok;
		};

		struct sBuffer
		{
		public:
			u8	*buffer;
			u32	numBytesInBuffer;
			u32	SIZE;

		public:
					sBuffer()														{ buffer = NULL; SIZE = 0; numBytesInBuffer = 0; }

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
			void	reset() { this->numBytesInBuffer = 0; }
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
		struct sFileUpload
		{
			FILE			*f;
			u32				totalFileSizeBytes;
			u16				packetSizeBytes;
			u32				rcvBytesSoFar;
			
		};

	private:
		void					priv_close ();
		void					priv_2280_accept();
		void					priv_2280_onIncomingData (OSSocket &sok, u32 uid);
		void					priv_2280_onClientDisconnected (OSSocket &sok, u32 uid);
		sConnectedSocket*		priv_2280_findClientByUID (u32 uid);

		void					priv_rs232_handleIncomingData (sBuffer &b);
		void					priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend);
		bool					priv_rs232_handleCommand_A (sBuffer &b);
		bool					priv_rs232_handleCommand_R (sBuffer &b);

		void					priv_identify_run();
		bool					priv_identify_waitAnswer(u8 command, u8 code, u8 len, u8 *answerBuffer, u32 timeoutMSec);

		void					priv_boot_run();
		void					priv_boot_rs232_handleCommunication (sBuffer &b);
		u32						priv_boot_buildMsgBuffer (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData);
		void					priv_boot_buildMsgBufferAndSend (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData);

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		rhea::NullLogger        nullLogger;
		OSSocket				sok2280;
		OSSerialPort			com;
		OSWaitableGrp			waitableGrp;
		u32						sok2280NextID;
		u8						*rs232BufferOUT;
		sBuffer					rs232BufferIN;
		u8						*sok2280Buffer;
		rhea::FastArray< sConnectedSocket>			clientList;
		u8						reportedESAPIVerMajor;
		u8						reportedESAPIVerMinor;
		esapi::eGPUType			reportedGPUType;
		bool					bQuit;
		u8						wifiIP[4];
        u8                      ssid[64];
		sFileUpload				fileUpload;
	};
} //namespace raspi

#endif //_raspiCore_h_
